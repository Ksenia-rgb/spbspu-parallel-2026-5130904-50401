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
