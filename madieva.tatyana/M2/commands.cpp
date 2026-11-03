#include "commands.hpp"
#include <string>
#include <stdexcept>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include "parser.hpp"
#include "file_descriptor.hpp"

void madieva::addCircle(madieva::state_t & state, const madieva::Args & args)
{
  if (args.size() != 5)
  {
    throw std::invalid_argument("incorrect number of arguments");
  }

  const std::string & name = args[1];
  checkName(name);
  const long long r = parseInteger(args[2]);
  const long long x = parseInteger(args[3]);
  const long long y = parseInteger(args[4]);

  Circle circle(r, x, y);

  auto it = state.circles.find(name);
  if (it != state.circles.end())
  {
    throw std::invalid_argument("the name of the circle exists");
  }
  state.circles.insert({name, circle});
}

void madieva::addSet(madieva::state_t & state, const madieva::Args & args)
{
  if (args.size() < 3)
  {
    throw std::invalid_argument("incorrect number of arguments");
  }

  const std::string & name = args[1];
  checkName(name);
  const long long count = parseInteger(args[2]);

  if (count <= 0)
  {
    throw std::invalid_argument("incorrect count");
  }

  if (args.size() != 3 + static_cast< std::size_t >(count))
  {
    throw std::invalid_argument("incorrect number of arguments");
  }

  const auto it_name = state.sets.find(name);
  if (it_name != state.sets.end())
  {
    throw std::invalid_argument("the name of the set exists");
  }

  std::vector< const Circle * > shapes;
  shapes.reserve(count);

  for (std::size_t i = 3; i < args.size(); ++i)
  {
    const std::string & circle_name = args[i];
    const auto it = state.circles.find(circle_name);
    if (it == state.circles.end())
    {
      throw std::invalid_argument("the circle name does not exist");
    }
    shapes.push_back(&it->second);
  }

  state.sets.insert({name, shapes});
}

void madieva::show(const madieva::state_t & state, const madieva::Args & args)
{
  if (args.size() != 2)
  {
    throw std::invalid_argument("incorrect number of arguments");
  }

  const std::string & name = args[1];
  const auto it = state.circles.find(name);
  if (it == state.circles.end())
  {
    throw std::invalid_argument("the name of the circle does not exist");
  }
  std::cout << it->second << "\n";
}

void madieva::showSet(const madieva::state_t & state, const madieva::Args & args)
{
  if (args.size() != 2)
  {
    throw std::invalid_argument("incorrect number of arguments");
  }

  const std::string & name = args[1];
  const auto it = state.sets.find(name);
  if (it == state.sets.end())
  {
    throw std::invalid_argument("the name of the set does not exist");
  }
  for (std::size_t i = 0; i < it->second.size(); ++i)
  {
    std::cout << *(it->second[i]) << "\n";
  }
}

void madieva::showFrame(const madieva::state_t & state, const madieva::Args & args)
{
  if (args.size() != 2)
  {
    throw std::invalid_argument("incorrect number of arguments");
  }

  const std::string & name = args[1];
  const auto it = state.circles.find(name);
  if (it == state.circles.end())
  {
    throw std::invalid_argument("the name of the circle does not exist");
  }
  std::cout << madieva::computeFrame(it->second) << "\n";
}

void madieva::showFrameSet(const madieva::state_t & state, const madieva::Args & args)
{
  if (args.size() != 2)
  {
    throw std::invalid_argument("incorrect number of arguments");
  }

  const std::string & name = args[1];
  const auto it = state.sets.find(name);
  if (it == state.sets.end())
  {
    throw std::invalid_argument("the name of the set does not exist");
  }
  std::cout << madieva::computeFrame(it->second) << "\n";
}

void madieva::spawn(madieva::state_t & state, const madieva::Args & args)
{
  if (args.size() != 3)
  {
    throw std::invalid_argument("incorrect number of arguments");
  }
  const std::string & process_name = args[1];
  checkName(process_name);
  const long long seed = parseInteger(args[2]);

  if (seed < 0)
  {
    throw std::invalid_argument("seed must be non-negative");
  }
  const auto it = state.processes.find(process_name);
  if (it == state.processes.end())
  {
    throw std::invalid_argument("process with this name already exists");
  }

  int cmd_pps[2] = {};
  int res_pps[2] = {};
  if (pipe(cmd_pps) != 0)
  { 
    throw std::runtime_error("pipe failed");
  }
  if (pipe(res_pps) != 0)
  {
    close(cmd_pps[0]);
    close(cmd_pps[1]);
    throw std::runtime_error("pipe failed");
  }

  FileDescriptor cmd_rd(cmd_pps[0]);
  FileDescriptor cmd_wr(cmd_pps[1]);
  FileDescriptor res_rd(res_pps[0]);
  FileDescriptor res_wr(res_pps[1]);

  pid_t pid = fork();

  if (pid < 0)
  {
    throw std::runtime_error("fork failed");
  }

  if (pid == 0) {
    cmd_wr.close();
    res_rd.close();
    std::string cmd_fd_s = std::to_string(cmd_rd.get());
    std::string res_fd_s = std::to_string(res_wr.get());
    std::string seed_s = std::to_string(seed);

    execl("./child", "child", cmd_fd_s.c_str(), res_fd_s.c_str(), seed_s.c_str(), nullptr);

    std::cerr << "exec failed: " << strerror(errno) << "\n";
    _exit(127);
  }

  cmd_rd.close();
  res_wr.close();

  state.processes.insert({process_name, Process{pid, std::move(cmd_wr), std::move(res_rd), seed}});
}

void madieva::area_on(madieva::state_t & state, const madieva::Args & args)
{
  if (args.size() != 6)
  {
    throw std::invalid_argument("incorrect number of arguments");
  }
  const std::string & process_name = args[1];
  const std::string & calc_name = args[2];
  const std::string & set_name = args[3];
  const long long threads = parseInteger(args[4]);
  const long long tries = parseInteger(args[5]);

  const auto proc_it = state.processes.find(process_name);
  if (proc_it == state.processes.end())
  {
    throw std::invalid_argument("process with this name does not exist");
  }

  const auto it = state.calc.find(calc_name);
  if (it != state.calc.end())
  {
    throw std::invalid_argument("calc with this name already exists");
  }

  if (state.sets.find(set_name) == state.sets.end())
  {
    throw std::invalid_argument("set with this name does not exist");
  }

  if (threads <= 0)
  {
    throw std::invalid_argument("the number of threads must be positive");
  }

  if (tries <= 0)
  {
    throw std::invalid_argument("the number of tries must be positive");
  }

//пайп

  state.calc.insert({calc_name, calc_t{process_name, threads, tries, true, 0.0, 0}});
}
