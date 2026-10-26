#include <utility>
#include <unistd.h>
#include <poll.h>
#include <iomanip>

#include "command.hpp"
#include "parse.hpp"

sedov::CommandTable sedov::makeCommandTable()
{
  CommandTable table;
  table.emplace("circle", cmdCircle);
  table.emplace("show", cmdShow);
  table.emplace("set", cmdSet);
  table.emplace("showset", cmdShowset);
  table.emplace("frame", cmdFrame);
  table.emplace("frameset", cmdFrameset);
  table.emplace("spawn", cmdSpawn);
  table.emplace("area-on", cmdAreaOn);
  table.emplace("wait", cmdWait);
  table.emplace("status", cmdStatus);
  return table;
}

void sedov::printCalcResult(const Calc & calc, std::ostream & out)
{
  if (calc.status == Status::InProgress) {
    out << calc.process_name << " <IN PROGRESS>\n";
  } else if (calc.status == Status::Canceled) {
    out << calc.process_name << " <CANCELED>\n";
  } else {
    out << calc.process_name << " " << calc.time_ms << " " << std::fixed << std::setprecision(3) << calc.area << "\n";
  }
}

bool sedov::tryReadResponse(CommandContext & ctx, int fd)
{
  Response resp{};
  if (!recvFull(fd, &resp, sizeof(resp))) {
    return false;
  }

  auto it = ctx.calc_id_to_name.find(resp.calc_id);
  if (it == ctx.calc_id_to_name.end()) {
    return false;
  }

  auto calc_it = ctx.calcs.find(it->second);
  if (calc_it != ctx.calcs.end()) {
    calc_it->second.status = Status::Done;
    calc_it->second.time_ms = resp.time_ms;
    calc_it->second.area = resp.area;
  }
  return true;
}

void sedov::cmdCircle(const std::vector< std::string > & words, CommandContext & ctx, std::ostream & out)
{
  if (words.size() != 5) {
    out << "<INVALID COMMAND>\n";
    return;
  }
  if (ctx.shapes.count(words[1]) > 0) {
    out << "<INVALID COMMAND>\n";
    return;
  }
  long long r, x, y;
  if (!parseLongLong(words[2], r) || !parseLongLong(words[3], x) || !parseLongLong(words[4], y)) {
    out << "<INVALID COMMAND>\n";
    return;
  }
  if (r <= 0) {
    out << "<INVALID COMMAND>\n";
    return;
  }
  ctx.shapes.emplace(words[1], Shape{r, x, y});
}

void sedov::cmdShow(const std::vector< std::string > & words, CommandContext & ctx, std::ostream & out)
{
  if (words.size() != 2) {
    out << "<INVALID COMMAND>\n";
    return;
  }
  auto it = ctx.shapes.find(words[1]);
  if (it == ctx.shapes.end()) {
    out << "<INVALID COMMAND>\n";
    return;
  }
  const Shape & s = it->second;
  out << s.r << " (" << s.x << " " << s.y << ")\n";
}

void sedov::cmdSet(const std::vector< std::string > & words, CommandContext & ctx, std::ostream & out)
{
  if (words.size() < 3) {
    out << "<INVALID COMMAND>\n";
    return;
  }
  long long n;
  if (!parseLongLong(words[2], n) || n <= 0) {
    out << "<INVALID COMMAND>\n";
    return;
  }
  if (words.size() != 3 + static_cast< size_t >(n)) {
    out << "<INVALID COMMAND>\n";
    return;
  }
  for (size_t i = 0; i < static_cast< size_t >(n); ++i) {
    if (ctx.shapes.count(words[3 + i]) == 0) {
      out << "<INVALID COMMAND>\n";
      return;
    }
  }
  if (ctx.sets.count(words[1]) > 0) {
    out << "<INVALID COMMAND>\n";
    return;
  }
  std::vector< std::string > v(words.begin() + 3, words.begin() + 3 + n);
  ctx.sets.emplace(words[1], std::move(v));
}

void sedov::cmdShowset(const std::vector< std::string > & words, CommandContext & ctx, std::ostream & out)
{
  if (words.size() != 2) {
    out << "<INVALID COMMAND>\n";
    return;
  }
  auto it = ctx.sets.find(words[1]);
  if (it == ctx.sets.end()) {
    out << "<INVALID COMMAND>\n";
    return;
  }
  for (const auto & name : it->second) {
    const Shape & s = ctx.shapes.at(name);
    out << s.r << " (" << s.x << " " << s.y << ")\n";
  }
}

void sedov::cmdFrame(const std::vector< std::string > & words, CommandContext & ctx, std::ostream & out)
{
  if (words.size() != 2) {
    out << "<INVALID COMMAND>\n";
    return;
  }
  auto it = ctx.shapes.find(words[1]);
  if (it == ctx.shapes.end()) {
    out << "<INVALID COMMAND>\n";
    return;
  }
  const Box box = boundingBox(it->second);
  out << "(" << box.x_min << " " << box.y_min << ") (" << box.x_max << " " << box.y_max << ")\n";
}

void sedov::cmdFrameset(const std::vector< std::string > & words, CommandContext & ctx, std::ostream & out)
{
  if (words.size() != 2) {
    out << "<INVALID COMMAND>\n";
    return;
  }
  auto it = ctx.sets.find(words[1]);
  if (it == ctx.sets.end()) {
    out << "<INVALID COMMAND>\n";
    return;
  }
  std::vector< Shape > shapes;
  shapes.reserve(it->second.size());
  for (const auto & name : it->second) {
    shapes.push_back(ctx.shapes.at(name));
  }
  const Box box = boundingBox(shapes);
  out << "(" << box.x_min << " " << box.y_min << ") (" << box.x_max << " " << box.y_max << ")\n";
}

void sedov::cmdSpawn(const std::vector< std::string > & words, CommandContext & ctx, std::ostream & out)
{
  if (words.size() != 3) {
    out << "<INVALID COMMAND>\n";
    return;
  }
  if (ctx.processes.count(words[1]) > 0) {
    out << "<INVALID COMMAND>\n";
    return;
  }
  long long seed;
  if (!parseLongLong(words[2], seed) || seed < 0) {
    out << "<INVALID COMMAND>\n";
    return;
  }
  int in_pipe[2] = {-1, -1};
  int out_pipe[2] = {-1, -1};
  if (pipe(in_pipe) != 0 || pipe(out_pipe) != 0) {
    out << "<INVALID COMMAND>\n";
    if (in_pipe[0] != -1) {
      close(in_pipe[0]);
      close(in_pipe[1]);
    }
    if (out_pipe[0] != -1) {
      close(out_pipe[0]);
      close(out_pipe[1]);
    }
    return;
  }
  pid_t pid = fork();
  if (pid < 0) {
    out << "<INVALID COMMAND>\n";
    close(in_pipe[0]);
    close(in_pipe[1]);
    close(out_pipe[0]);
    close(out_pipe[1]);
    return;
  }
  if (!pid) {
    close(in_pipe[1]);
    close(out_pipe[0]);
    childLoop(in_pipe[0], out_pipe[1], static_cast< size_t >(seed));
    _exit(0);
  }
  close(in_pipe[0]);
  close(out_pipe[1]);
  ctx.processes.emplace(words[1], Process{pid, in_pipe[1], out_pipe[0], true});
}
