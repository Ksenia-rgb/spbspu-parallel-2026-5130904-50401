#include "commands.hpp"

#include <iostream>
#include <istream>
#include <limits>
#include <unistd.h>
#include <sys/wait.h>
#include <cerrno>

std::istream &chernov::operator>>(std::istream &input, const CommandIO &cmd_io)
{
  std::istream::sentry sentry(input);
  if (!sentry) {
    return input;
  }

  std::string str = "";
  input >> str;

  if (input) {
    try {
      cmd_io.cmds.at(str)(input, cmd_io.output, cmd_io.app);
    } catch (...) {
      input.setstate(std::ios::failbit);
    }
  }

  if (!input) {
    input.clear();
    input.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
    if (cmd_io.cmds.find(str) != cmd_io.cmds.end()) {
      cmd_io.output << "<INVALID COMMAND>\n";
    } else {
      cmd_io.output << "<UNKNOWN COMMAND>\n";
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

  if (app.ellipses.find(name) != app.ellipses.end()) {
    throw std::runtime_error("ellipse name already exists");
  }
  if (r <= 0) {
    throw std::runtime_error("ellipse radius must be greater than zero");
  }

  app.ellipses.insert(
      {name, ellipse_t{point_t{x, y}, r, r, 0}});
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

  app.sets.insert({name, std::move(set)});
}

void chernov::cmdShow(std::istream &input, std::ostream &output, App &app)
{
  std::string name;
  if (!(input >> name)) {
    throw std::runtime_error("invalid input");
  }

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
  size_t seed;
  if (!(input >> process_name >> seed)) {
    throw std::runtime_error("invalid input");
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

    calculationLoop(to_child[0], to_parent[1], seed);
  } else {
    close(to_child[0]);
    close(to_parent[1]);

    process_t process{pid, seed, to_child[1], to_parent[0], ProcessState::Active, "", {}};

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

  calc_t calc{process_name, TaskState::Pending, 0, 0, app.id_counter, std::move(ellipses), static_cast< size_t >(threads),
      static_cast< size_t >(tries)};

  app.calcs.insert({calc_name, std::move(calc)});
  try {
    process.pending_calcs.push_back(calc_name);
  } catch (...) {
    app.calcs.erase(calc_name);
    throw;
  }

  ++app.id_counter;
}

void chernov::cmdStatus(std::istream &input, std::ostream &, App &app)
{
  std::string calc_name;
  if (!(input >> calc_name)) {
    throw std::runtime_error("invalid input");
  }

  if (app.calcs.find(calc_name) == app.calcs.end()) {
    throw std::runtime_error("calc name not found");
  }
}

void chernov::cmdWait(std::istream &input, std::ostream &, App &app)
{
  std::string calc_name;
  if (!(input >> calc_name)) {
    throw std::runtime_error("invalid input");
  }

  if (app.calcs.find(calc_name) == app.calcs.end()) {
    throw std::runtime_error("calc name not found");
  }
}
