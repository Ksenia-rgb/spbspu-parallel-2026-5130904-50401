#include "commands.hpp"

#include <algorithm>
#include <chrono>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <istream>
#include <limits>
#include <poll.h>
#include <streambuf>
#include <unistd.h>
#include <sys/wait.h>
#include <cerrno>

namespace chernov {
  namespace detail {
    class CommandBuffer: public std::streambuf {
    public:
      explicit CommandBuffer(std::string &line)
      {
        if (line.empty()) {
          setg(nullptr, nullptr, nullptr);
        } else {
          char *begin = &line[0];
          setg(begin, begin, begin + line.size());
        }
      }
    };

    bool isValidName(const std::string &name)
    {
      return !name.empty()
          && ((name.front() >= 'a' && name.front() <= 'z') || (name.front() >= 'A' && name.front() <= 'Z'));
    }

    void requireEnd(std::istream &input)
    {
      input >> std::ws;
      if (!input.eof()) {
        throw std::runtime_error("extra arguments");
      }
    }
  }
}

std::istream &chernov::operator>>(std::istream &input, const CommandIO &cmd_io)
{
  std::istream::sentry sentry(input);
  if (!sentry) {
    return input;
  }

  std::string command;
  input >> command;
  std::string arguments;
  std::getline(input, arguments);
  detail::CommandBuffer buffer(arguments);
  std::istream command_input(&buffer);

  const auto command_it = cmd_io.cmds.find(command);
  if (command_it == cmd_io.cmds.end()) {
    cmd_io.output << "<UNKNOWN COMMAND>\n";
  } else {
    try {
      command_it->second(command_input, cmd_io.output, cmd_io.app);
    } catch (...) {
      cmd_io.output << "<INVALID COMMAND>\n";
    }
  }

  return input;
}

void chernov::runCommands(std::istream &input, CommandIO &cmd_io)
{
  if (!(input >> cmd_io)) {
    return;
  }
  runCommands(input, cmd_io);
}

void chernov::cmdCircle(std::istream &input, std::ostream &, App &app)
{
  std::string name;
  double r, x, y;
  if (!(input >> name >> r >> x >> y)) {
    throw std::runtime_error("invalid input");
  }
  detail::requireEnd(input);

  if (!detail::isValidName(name) || app.ellipses.find(name) != app.ellipses.end()) {
    throw std::runtime_error("ellipse name already exists");
  }
  if (r <= 0) {
    throw std::runtime_error("ellipse radius must be greater than zero");
  }

  app.ellipses.insert({name, ellipse_t{point_t{x, y}, r, r, 0, true}});
}

void chernov::cmdEllipse(std::istream &input, std::ostream &, App &app)
{
  std::string name;
  double horizontal_radius, vertical_radius, x, y;
  if (!(input >> name >> horizontal_radius >> vertical_radius >> x >> y)) {
    throw std::runtime_error("invalid input");
  }
  detail::requireEnd(input);

  if (!detail::isValidName(name) || app.ellipses.find(name) != app.ellipses.end()) {
    throw std::runtime_error("ellipse name already exists");
  }
  if (horizontal_radius <= 0 || vertical_radius <= 0) {
    throw std::runtime_error("ellipse radii must be greater than zero");
  }

  app.ellipses.insert({name, ellipse_t{point_t{x, y}, horizontal_radius, vertical_radius, 0, false}});
}

void chernov::cmdRotate(std::istream &input, std::ostream &, App &app)
{
  std::string name;
  double ksi;
  if (!(input >> name >> ksi)) {
    throw std::runtime_error("invalid input");
  }
  detail::requireEnd(input);

  if (app.ellipses.find(name) == app.ellipses.end()) {
    throw std::runtime_error("ellipse name not found");
  }

  app.ellipses.at(name).ksi += ksi;
}

void chernov::cmdSet(std::istream &input, std::ostream &, App &app)
{
  std::string name;
  long long size;
  if (!(input >> name >> size)) {
    throw std::runtime_error("invalid input");
  }

  if (app.sets.find(name) != app.sets.end()) {
    throw std::runtime_error("set name already exists");
  }
  if (size <= 0) {
    throw std::runtime_error("set size must be greater than zero");
  }

  set_t set;
  set.reserve(size);
  std::string shape;
  for (long long i = 0; i < size; ++i) {
    if (!(input >> shape)) {
      throw std::runtime_error("invalid input");
    }
    if (app.ellipses.find(shape) == app.ellipses.end()) {
      throw std::runtime_error("shape name not found");
    }

    set.push_back(shape);
  }
  detail::requireEnd(input);

  if (!detail::isValidName(name)) {
    throw std::runtime_error("invalid set name");
  }
  app.sets.insert({name, std::move(set)});
}

void chernov::cmdShow(std::istream &input, std::ostream &output, App &app)
{
  std::string name;
  if (!(input >> name)) {
    throw std::runtime_error("invalid input");
  }
  detail::requireEnd(input);

  if (app.ellipses.find(name) == app.ellipses.end()) {
    throw std::runtime_error("ellipse name not found");
  }

  output << app.ellipses.at(name) << "\n";
}

void chernov::cmdShowSet(std::istream &input, std::ostream &output, App &app)
{
  std::string name;
  if (!(input >> name)) {
    throw std::runtime_error("invalid input");
  }
  detail::requireEnd(input);

  if (app.sets.find(name) == app.sets.end()) {
    throw std::runtime_error("set name not found");
  }

  const set_t &set = app.sets.at(name);
  for (std::string ellipse_name : set) {
    output << app.ellipses.at(ellipse_name) << "\n";
  }
}

void chernov::cmdFrame(std::istream &input, std::ostream &output, App &app)
{
  std::string name;
  if (!(input >> name)) {
    throw std::runtime_error("invalid input");
  }
  detail::requireEnd(input);

  if (app.ellipses.find(name) == app.ellipses.end()) {
    throw std::runtime_error("ellipse name not found");
  }

  const frame_t frame = getFrame(app.ellipses.at(name));
  output << frame << "\n";
}

void chernov::cmdFrameSet(std::istream &input, std::ostream &output, App &app)
{
  std::string name;
  if (!(input >> name)) {
    throw std::runtime_error("invalid input");
  }
  detail::requireEnd(input);

  if (app.sets.find(name) == app.sets.end()) {
    throw std::runtime_error("set name not found");
  }

  const set_t &set = app.sets.at(name);
  std::vector< ellipse_t > ellipses_vector;
  ellipses_vector.reserve(set.size());

  for (std::string ellipse_name : set) {
    ellipses_vector.push_back(app.ellipses.at(ellipse_name));
  }

  const frame_t frame = getFrame(ellipses_vector);
  output << frame << "\n";
}

void chernov::cmdSpawn(std::istream &input, std::ostream &, App &app)
{
  std::string process_name;
  long long seed;
  if (!(input >> process_name >> seed)) {
    throw std::runtime_error("invalid input");
  }
  detail::requireEnd(input);

  if (!detail::isValidName(process_name) || seed < 0) {
    throw std::runtime_error("seed must not be negative");
  }

  if (app.processes.find(process_name) != app.processes.end()) {
    throw std::runtime_error("process name already exists");
  }

  int to_child[2];
  int to_parent[2];

  if (pipe(to_child) == -1) {
    throw std::runtime_error("pipe error");
  }

  if (pipe(to_parent) == -1) {
    close(to_child[0]);
    close(to_child[1]);
    throw std::runtime_error("pipe error");
  }

  const pid_t pid = fork();

  if (pid == -1) {
    close(to_child[0]);
    close(to_child[1]);
    close(to_parent[0]);
    close(to_parent[1]);
    throw std::runtime_error("fork failed");
  }
  if (pid == 0) {
    close(to_child[1]);
    close(to_parent[0]);
    for (const auto &process : app.processes) {
      if (process.second.result_fd >= 0) {
        close(process.second.result_fd);
      }
      if (process.second.task_fd >= 0) {
        close(process.second.task_fd);
      }
    }

    calculationLoop(to_child[0], to_parent[1], static_cast< size_t >(seed));
  } else {
    close(to_child[0]);
    close(to_parent[1]);

    const int flags = fcntl(to_parent[0], F_GETFL);

    if (flags == -1 || fcntl(to_parent[0], F_SETFL, flags | O_NONBLOCK) == -1) {
      close(to_child[1]);
      close(to_parent[0]);
      pid_t result;
      do {
        result = waitpid(pid, nullptr, 0);
      } while (result == -1 && errno == EINTR);

      throw std::runtime_error("fcntl failed");
    }

    process_t process{pid, static_cast< size_t >(seed), to_child[1], to_parent[0], ProcessState::Active, "", {}};

    try {
      app.processes.insert({process_name, std::move(process)});
    } catch (...) {
      close(to_child[1]);
      close(to_parent[0]);

      int result;
      do {
        result = waitpid(pid, nullptr, 0);
      } while (result == -1 && errno == EINTR);
      throw;
    }
  }
}

void chernov::cmdAreaOn(std::istream &input, std::ostream &, App &app)
{
  std::string process_name, calc_name, set_name;
  long long threads, tries;
  if (!(input >> process_name >> calc_name >> set_name >> threads >> tries)) {
    throw std::runtime_error("invalid input");
  }
  detail::requireEnd(input);

  if (!detail::isValidName(calc_name)) {
    throw std::runtime_error("invalid calc name");
  }

  if (app.processes.find(process_name) == app.processes.end()) {
    throw std::runtime_error("process name not found");
  }

  if (app.calcs.find(calc_name) != app.calcs.end()) {
    throw std::runtime_error("calc name already exists");
  }

  if (app.sets.find(set_name) == app.sets.end()) {
    throw std::runtime_error("set name not found");
  }

  if (threads <= 0) {
    throw std::runtime_error("threads must be positive");
  }

  if (tries <= 0) {
    throw std::runtime_error("tries must be positive");
  }

  process_t &process = app.processes.at(process_name);
  if (process.state != ProcessState::Active) {
    throw std::runtime_error("process state must be active");
  }

  if (app.id_counter == std::numeric_limits< size_t >::max()) {
    throw std::runtime_error("calculation id limit reached");
  }

  std::vector< ellipse_t > ellipses;
  ellipses.reserve(app.sets.at(set_name).size());
  for (const std::string &ellipse_name : app.sets.at(set_name)) {
    ellipses.push_back(app.ellipses.at(ellipse_name));
  }

  calc_t calc{process_name, TaskState::Pending, 0, 0, app.id_counter, std::move(ellipses),
      static_cast< size_t >(threads), static_cast< size_t >(tries)};

  app.calcs.insert({calc_name, std::move(calc)});
  try {
    process.pending_calcs.push_back(calc_name);
  } catch (...) {
    app.calcs.erase(calc_name);
    throw;
  }

  ++app.id_counter;
  startNextCalculation(app, process);
}

void chernov::cmdStatus(std::istream &input, std::ostream &output, App &app)
{
  std::string calc_name;
  if (!(input >> calc_name)) {
    throw std::runtime_error("invalid input");
  }
  detail::requireEnd(input);

  if (app.calcs.find(calc_name) == app.calcs.end()) {
    throw std::runtime_error("calc name not found");
  }

  checkProcesses(app);
  detail::printCalcStatus(output, app.calcs.at(calc_name));
}

void chernov::cmdWait(std::istream &input, std::ostream &output, App &app)
{
  std::string calc_name;
  if (!(input >> calc_name)) {
    throw std::runtime_error("invalid input");
  }
  detail::requireEnd(input);

  if (app.calcs.find(calc_name) == app.calcs.end()) {
    throw std::runtime_error("calc name not found");
  }

  checkProcesses(app);
  calc_t &calc = app.calcs.at(calc_name);
  while (calc.state == TaskState::Pending || calc.state == TaskState::Running) {
    process_t &process = app.processes.at(calc.process_name);
    if (process.result_fd < 0) {
      throw std::runtime_error("result pipe is closed");
    }

    pollfd descriptor{process.result_fd, POLLIN, 0};
    int result;
    do {
      result = poll(&descriptor, 1, -1);
    } while (result == -1 && errno == EINTR);
    if (result == -1) {
      throw std::runtime_error("poll failed");
    }

    checkProcesses(app);
  }

  detail::printCalcStatus(output, calc);
}

void chernov::cmdTimeout(std::istream &input, std::ostream &output, App &app)
{
  std::string calc_name;
  long long timeout_ms;
  if (!(input >> calc_name >> timeout_ms)) {
    throw std::runtime_error("invalid input");
  }
  detail::requireEnd(input);

  if (app.calcs.find(calc_name) == app.calcs.end()) {
    throw std::runtime_error("calc name not found");
  }
  if (timeout_ms < 0) {
    throw std::runtime_error("timeout must not be negative");
  }

  checkProcesses(app);
  calc_t &calc = app.calcs.at(calc_name);
  const auto start = std::chrono::steady_clock::now();
  while (calc.state == TaskState::Pending || calc.state == TaskState::Running) {
    const auto elapsed
        = std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::steady_clock::now() - start);
    if (elapsed.count() >= timeout_ms) {
      break;
    }

    process_t &process = app.processes.at(calc.process_name);
    if (process.result_fd < 0) {
      throw std::runtime_error("result pipe is closed");
    }

    const long long remaining = timeout_ms - elapsed.count();
    const int poll_timeout
        = static_cast< int >(std::min(remaining, static_cast< long long >(std::numeric_limits< int >::max())));
    pollfd descriptor{process.result_fd, POLLIN, 0};
    const int result = poll(&descriptor, 1, poll_timeout);
    if (result == -1) {
      if (errno == EINTR) {
        continue;
      }
      throw std::runtime_error("poll failed");
    }

    checkProcesses(app);
  }

  detail::printCalcStatus(output, calc);
}

void chernov::cmdKill(std::istream &input, std::ostream &, App &app)
{
  std::string process_name;
  if (!(input >> process_name)) {
    throw std::runtime_error("invalid input");
  }
  detail::requireEnd(input);

  if (app.processes.find(process_name) == app.processes.end()) {
    throw std::runtime_error("process name not found");
  }

  process_t &process = app.processes.at(process_name);
  if (process.state != ProcessState::Active) {
    throw std::runtime_error("process state must be active");
  }

  process.state = ProcessState::Stopping;
  for (const std::string &calc_name : process.pending_calcs) {
    calc_t &calc = app.calcs.at(calc_name);
    if (calc.state == TaskState::Pending) {
      calc.state = TaskState::Cancelled;
    }
  }
  process.pending_calcs.clear();
  if (process.task_fd >= 0) {
    close(process.task_fd);
    process.task_fd = -1;
  }

  pid_t result;
  do {
    result = waitpid(process.pid, nullptr, 0);
  } while (result == -1 && errno == EINTR);
  if (result == -1 && errno != ECHILD) {
    throw std::runtime_error("waitpid failed");
  }

  if (process.result_fd >= 0) {
    try {
      receiveAvailableResult(app, process);
    } catch (...) {
    }
  }
  if (!process.current_calc_name.empty()) {
    calc_t &calc = app.calcs.at(process.current_calc_name);
    if (calc.state == TaskState::Running) {
      calc.state = TaskState::Failed;
    }
  }
  if (process.result_fd >= 0) {
    close(process.result_fd);
  }
  process.result_fd = -1;
  process.buffer.clear();
  process.current_calc_name.clear();
  process.state = ProcessState::Stopped;
}

void chernov::detail::printCalcStatus(std::ostream &output, const calc_t &calc)
{
  IOguard guard(output);
  output << calc.process_name << " ";
  if (calc.state == TaskState::Completed) {
    output << std::fixed << std::setprecision(3) << calc.milliseconds << " " << calc.result;
  } else if (calc.state == TaskState::Cancelled) {
    output << "<CANCELED>";
  } else if (calc.state == TaskState::Failed) {
    output << "<FAILED>";
  } else {
    output << "<IN PROGRESS>";
  }
  output << "\n";
}
