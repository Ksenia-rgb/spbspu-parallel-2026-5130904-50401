#include <string>
#include <cctype>
#include <vector>
#include <utility>
#include <unistd.h>
#include <system_error>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <poll.h>
#include "commands.hpp"
#include "ipc.hpp"

void vasyakin::cmd_circle(std::istream& in, std::ostream&, vasyakin::State& state)
{
  std::string circle_name;
  int r = 0, x = 0, y = 0;

  if (!(in >> circle_name >> r >> x >> y))
  {
    throw std::invalid_argument("invalid command");
  }

  if (circle_name.empty() || !std::isalpha(static_cast< unsigned char >(circle_name[0])))
  {
    throw std::invalid_argument("invalid command");
  }

  if (r <= 0)
  {
    throw std::invalid_argument("invalid command");
  }

  if (state.circles.count(circle_name))
  {
    throw std::invalid_argument("invalid command");
  }

  state.circles.insert({circle_name, circle_t{r, x, y}});
}

void vasyakin::cmd_set(std::istream& in, std::ostream&, vasyakin::State& state)
{
  std::string set_name;
  size_t set_size = 0;

  if (!(in >> set_name >> set_size))
  {
    throw std::invalid_argument("invalid command");
  }

  if (set_name.empty() || !std::isalpha(static_cast< unsigned char >(set_name[0])))
  {
    throw std::invalid_argument("invalid command");
  }

  if (set_size == 0)
  {
    throw std::invalid_argument("invalid command");
  }

  if (state.sets.count(set_name))
  {
    throw std::invalid_argument("invalid command");
  }

  vasyakin::ShapeSet ss;

  for (size_t i = 0; i < set_size; ++i)
  {
    std::string shape_name;
    if (!(in >> shape_name))
    {
      throw std::invalid_argument("invalid command");
    }

    if (!state.circles.count(shape_name))
    {
      throw std::invalid_argument("invalid command");
    }

    ss.shape_names.push_back(shape_name);
  }

  state.sets.insert({set_name, ss});
}

void vasyakin::cmd_show(std::istream& in, std::ostream& out, vasyakin::State& state)
{
  std::string shape_name;

  if (!(in >> shape_name))
  {
    throw std::invalid_argument("invalid command");
  }

  if (!state.circles.count(shape_name))
  {
    throw std::invalid_argument("invalid command");
  }

  const circle_t& circle = state.circles[shape_name];
  out << circle.r << " (" << circle.x << " " << circle.y << ")" << '\n';
}

void vasyakin::cmd_showset(std::istream& in, std::ostream& out, vasyakin::State& state)
{
  std::string set_name;

  if (!(in >> set_name))
  {
    throw std::invalid_argument("invalid command");
  }

  if (!state.sets.count(set_name))
  {
    throw std::invalid_argument("invalid command");
  }

  for (size_t i = 0; i < state.sets[set_name].shape_names.size(); ++i)
  {
    const circle_t& circle = state.circles.at(state.sets[set_name].shape_names[i]);
    out << circle.r << " (" << circle.x << " " << circle.y << ")" << '\n';
  }
}

void vasyakin::cmd_frame(std::istream& in, std::ostream& out, vasyakin::State& state)
{
  std::string shape_name;

  if (!(in >> shape_name))
  {
    throw std::invalid_argument("invalid command");
  }

  if (!state.circles.count(shape_name))
  {
    throw std::invalid_argument("invalid command");
  }

  const circle_t& circle = state.circles[shape_name];

  std::pair< int, int > right = std::make_pair(circle.x + circle.r, circle.y + circle.r);
  std::pair< int, int > left = std::make_pair(circle.x - circle.r, circle.y - circle.r);

  out << "(" << left.first << " " << left.second << ")" << " (" << right.first << " " << right.second << ")" << '\n';
}

void vasyakin::cmd_frameset(std::istream& in, std::ostream& out, vasyakin::State& state)
{
  std::string set_name;

  if (!(in >> set_name))
  {
    throw std::invalid_argument("invalid command");
  }

  if (!state.sets.count(set_name))
  {
    throw std::invalid_argument("invalid command");
  }

  const ShapeSet& set = state.sets[set_name];
  if (set.shape_names.empty())
  {
    throw std::invalid_argument("invalid command");
  }

  const circle_t& circle_first = state.circles.at(set.shape_names[0]);

  std::pair< int, int > right = std::make_pair(circle_first.x + circle_first.r, circle_first.y + circle_first.r);
  std::pair< int, int > left = std::make_pair(circle_first.x - circle_first.r, circle_first.y - circle_first.r);

  for (size_t i = 1; i < set.shape_names.size(); ++i)
  {
    const circle_t& c = state.circles.at(set.shape_names[i]);

    right = {std::max(right.first, c.x + c.r), std::max(right.second, c.y + c.r)};
    left = {std::min(left.first, c.x - c.r), std::min(left.second, c.y - c.r)};
  }

  out << "(" << left.first << " " << left.second << ")" << " (" << right.first << " " << right.second << ")" << '\n';
}

void vasyakin::cmd_spawn(std::istream& in, std::ostream&, vasyakin::State& state)
{
  std::string name;
  size_t seed = 0;
  if (!(in >> name >> seed))
  {
    throw std::invalid_argument("invalid command");
  }

  if (name.empty() || !std::isalpha(static_cast< unsigned char >(name[0])))
  {
    throw std::invalid_argument("invalid command");
  }

  if (state.processes.count(name))
  {
    throw std::invalid_argument("invalid command");
  }

  int cmd_pipe[2] = {};
  int res_pipe[2] = {};

  int err = pipe(cmd_pipe);
  if (err == -1)
  {
    throw std::system_error(errno, std::generic_category(), "Failed to create pipe");
  }

  err = pipe(res_pipe);
  if (err == -1)
  {
    close(cmd_pipe[0]);
    close(cmd_pipe[1]);
    throw std::system_error(errno, std::generic_category(), "Failed to create pipe");
  }

  int cmd_rd = cmd_pipe[0];
  int cmd_wr = cmd_pipe[1];

  int res_rd = res_pipe[0];
  int res_wr = res_pipe[1];

  pid_t pid = fork();
  if (pid < 0)
  {
    close(cmd_rd);
    close(cmd_wr);

    close(res_rd);
    close(res_wr);

    throw std::system_error(errno, std::generic_category(), "Fork failed");
  }
  else if (pid == 0)
  {
    for (const auto& entry : state.processes)
    {
      close(entry.second.rd_fd);
      close(entry.second.wr_fd);
    }

    close(cmd_wr);
    close(res_rd);

    if (setenv("CMD_RD", std::to_string(cmd_rd).c_str(), 1) != 0)
    {
      _exit(1);
    }

    if (setenv("RES_WR", std::to_string(res_wr).c_str(), 1) != 0)
    {
      _exit(1);
    }

    if (setenv("SEED", std::to_string(seed).c_str(), 1) != 0)
    {
      _exit(1);
    }

    execl(state.exe_path.c_str(), state.exe_path.c_str(), NULL);
    _exit(1);
  }
  else
  {
    close(cmd_rd);
    close(res_wr);

    ProcessInfo info;
    info.pid = pid;
    info.rd_fd = res_rd;
    info.wr_fd = cmd_wr;
    info.alive = true;
    info.seed = static_cast< unsigned int >(seed);

    state.processes.insert({name, info});
  }
}

void vasyakin::cmd_areaon(std::istream& in, std::ostream&, vasyakin::State& state)
{
  std::string process_name, calc_name, set_name;
  size_t threads = 0, tries = 0;

  if (!(in >> process_name >> calc_name >> set_name >> threads >> tries))
  {
    throw std::invalid_argument("invalid command");
  }

  if (process_name.empty() || !std::isalpha(static_cast< unsigned char >(process_name[0])))
  {
    throw std::invalid_argument("invalid command");
  }

  if (calc_name.empty() || !std::isalpha(static_cast< unsigned char >(calc_name[0])))
  {
    throw std::invalid_argument("invalid command");
  }

  if (set_name.empty() || !std::isalpha(static_cast< unsigned char >(set_name[0])))
  {
    throw std::invalid_argument("invalid command");
  }

  if (state.calculations.count(calc_name))
  {
    throw std::invalid_argument("invalid command");
  }

  auto proc_it = state.processes.find(process_name);
  if (proc_it == state.processes.end())
  {
    throw std::invalid_argument("invalid command");
  }

  if (!proc_it->second.alive)
  {
    throw std::invalid_argument("invalid command");
  }

  auto set_it = state.sets.find(set_name);
  if (set_it == state.sets.end())
  {
    throw std::invalid_argument("invalid command");
  }

  if (threads == 0 || tries == 0)
  {
    throw std::invalid_argument("invalid command");
  }

  const vasyakin::ShapeSet& set = set_it->second;

  std::vector< circle_t > circles;
  circles.reserve(set.shape_names.size());

  for (size_t i = 0; i < set.shape_names.size(); ++i)
  {
    circles.push_back(state.circles.at(set.shape_names[i]));
  }

  vasyakin::CommandHeader header;

  header.type = CommandType::CALCULATION;
  std::memset(header.calc_name, 0, MAX_NAME_LEN);
  std::strncpy(header.calc_name, calc_name.c_str(), MAX_NAME_LEN - 1);
  header.shape_count = circles.size();
  header.threads = threads;
  header.tries = tries;

  int err = 0;
  int wr_fd = proc_it->second.wr_fd;

  size_t sent = vasyakin::send(err, wr_fd, reinterpret_cast< char* >(&header), sizeof(header));

  if (sent != sizeof(header))
  {
    throw std::invalid_argument("invalid command");
  }

  size_t i = 0;
  while (i < circles.size())
  {
    vasyakin::ShapeBatch batch;
    size_t batch_count = std::min(BATCH_SIZE, circles.size() - i);
    batch.count = batch_count;

    for (size_t j = 0; j < batch_count; ++j)
    {
      batch.shapes[j] = circles[i + j];
    }

    sent = vasyakin::send(err, wr_fd, reinterpret_cast< char* >(&batch), sizeof(batch));
    if (sent != sizeof(batch))
    {
      throw std::invalid_argument("invalid command");
    }

    i += batch_count;
  }

  vasyakin::Calculation calculation;

  calculation.status = CalcStatus::IN_PROGRESS;
  calculation.process_name = process_name;
  calculation.area = 0.0;
  calculation.intersection_area = 0.0;
  calculation.time_ms = 0;

  state.calculations.insert({calc_name, calculation});
}

void vasyakin::cmd_status(std::istream& in, std::ostream& out, vasyakin::State& state)
{
  std::string calc_name;
  if (!(in >> calc_name))
  {
    throw std::invalid_argument("invalid command");
  }

  if (!state.calculations.count(calc_name))
  {
    throw std::invalid_argument("invalid command");
  }

  auto calc_it = state.calculations.find(calc_name);
  vasyakin::Calculation& calc = calc_it->second;

  if (calc.status == vasyakin::CalcStatus::COMPLETED)
  {
    out << calc.process_name << " " << calc.time_ms << " " << calc.area << " " << calc.intersection_area << '\n';
    return;
  }
  
  auto proc_it = state.processes.find(calc.process_name);
  if (proc_it == state.processes.end())
  {
    throw std::invalid_argument("invalid command");
  }

  struct pollfd pfd;
  pfd.fd = proc_it->second.rd_fd;
  pfd.events = POLLIN;

  int ret = poll(&pfd, 1, 0);
  if (ret < 0)
  {
    throw std::runtime_error(std::string("poll error: ") + std::strerror(errno));
  }

  if (ret > 0 && (pfd.revents & POLLIN))
  {
    vasyakin::Result result;
    int err = 0;

    size_t rec = vasyakin::recv(err, proc_it->second.rd_fd, reinterpret_cast< char* >(&result), sizeof(result));
    if (err != 0)
    {
      throw std::runtime_error("IPC error: " + std::string(std::strerror(err)));
    }

    if (rec == sizeof(result))
    {
      if (std::string(result.calc_name) == calc_name)
      {
        calc.area = result.area;
        calc.intersection_area = result.intersection_area;
        calc.time_ms = result.time_ms;
        calc.status = vasyakin::CalcStatus::COMPLETED;

        out << calc.process_name << " " << calc.time_ms << " " << calc.area << " " << calc.intersection_area << '\n';
        return;
      }
    }
  }

  out << "<IN PROGRESS>" << '\n';
}
