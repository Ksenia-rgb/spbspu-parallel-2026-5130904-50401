#include "commands.hpp"
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <deque>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include <sys/types.h>
#include <poll.h>
#include <utility>
#include "child.hpp"
#include "ipc.hpp"
#include "shared_circles.hpp"

namespace goltsov
{
  namespace
  {
    struct Process
    {
      std::string name;
      int wr;
      int rd;
      size_t seed;
      pid_t pid;
      std::deque< std::string > pending;
    };

    struct Calculation
    {
      std::string process;
      bool done;
      size_t ms;
      double area;
    };

    std::map< std::string, size_t > name_idx_map;
    std::map< std::string, std::vector< std::string > > sets;
    std::map< std::string, Process > processes;
    std::map< std::string, Calculation > calculations;

    void check(bool condition)
    {
      if (!condition)
      {
        throw std::runtime_error("<INVALID COMMAND>");
      }
    }

    bool isValidName(const std::string& name)
    {
      return !name.empty() && std::isalpha(static_cast< unsigned char >(name[0]));
    }

    std::vector< std::string > read_args()
    {
      std::string line;
      std::getline(std::cin, line);
      std::vector< std::string > args;
      const char* const spaces = " \t\r";
      size_t pos = 0;
      while (pos < line.size())
      {
        pos = line.find_first_not_of(spaces, pos);
        if (pos == std::string::npos)
        {
          break;
        }
        size_t end = line.find_first_of(spaces, pos);
        if (end == std::string::npos)
        {
          end = line.size();
        }
        args.push_back(line.substr(pos, end - pos));
        pos = end;
      }
      return args;
    }

    double toDouble(const std::string& token)
    {
      size_t pos = 0;
      double value = std::stod(token, &pos);
      check(pos == token.size() && std::isfinite(value));
      return value;
    }

    long long toInteger(const std::string& token)
    {
      size_t pos = 0;
      long long value = std::stoll(token, &pos);
      check(pos == token.size());
      return value;
    }

    void readResult(Process& process)
    {
      size_t ms = 0;
      recvValue(process.rd, ms);
      double res = NAN;
      recvValue(process.rd, res);
      Calculation& calculation = calculations.at(process.pending.front());
      calculation.ms = ms;
      calculation.area = res;
      calculation.done = true;
      process.pending.pop_front();
    }

    bool isReady(const Process& process)
    {
      pollfd pfd{process.rd, POLLIN, 0};
      return poll(&pfd, 1, 0) > 0;
    }

    void update(Process& process)
    {
      while (!process.pending.empty() && isReady(process))
      {
        readResult(process);
      }
    }

    void printResult(const Process& process, const Calculation& calculation)
    {
      printf("%s %zu %.3f\n", process.name.c_str(), calculation.ms, calculation.area);
    }
  }
}

void goltsov::circle(SharedCircles& shared_circles)
{
  std::vector< std::string > args = read_args();
  check(args.size() == 4);
  const std::string& name = args[0];
  double r = toDouble(args[1]);
  double x = toDouble(args[2]);
  double y = toDouble(args[3]);
  check(isValidName(name) && r > 0 && !name_idx_map.count(name));
  const size_t idx = shared_circles.getCountCircles();
  shared_circles.newCircle(r, x, y);
  name_idx_map.insert({name, idx});
}

void goltsov::set(SharedCircles&)
{
  std::vector< std::string > args = read_args();
  check(args.size() >= 2);
  const std::string& name = args[0];
  long long set_size = toInteger(args[1]);
  check(isValidName(name) && set_size > 0 && !sets.count(name));
  check(args.size() - 2 == static_cast< size_t >(set_size));
  std::vector< std::string > shapes;
  for (size_t i = 2; i < args.size(); ++i)
  {
    check(name_idx_map.count(args[i]));
    shapes.push_back(args[i]);
  }
  sets[name] = shapes;
}

void goltsov::show(SharedCircles& shared_circles)
{
  std::vector< std::string > args = read_args();
  check(args.size() == 1);
  const Circle* shape = shared_circles.getData() + name_idx_map.at(args[0]);
  std::cout << *shape << "\n";
}

void goltsov::showset(SharedCircles& shared_circles)
{
  std::vector< std::string > args = read_args();
  check(args.size() == 1);
  std::vector< std::string > set = sets.at(args[0]);
  for (size_t i = 0; i < set.size(); ++i)
  {
    std::cout << *(shared_circles.getData() + name_idx_map.at(set[i])) << "\n";
  }
}

void goltsov::frame(SharedCircles& shared_circles)
{
  std::vector< std::string > args = read_args();
  check(args.size() == 1);
  const Circle* shape = shared_circles.getData() + name_idx_map.at(args[0]);
  std::cout << "(" << shape->x - shape->r << " " << shape->y - shape->r << ") (" << shape->x + shape->r << " "
            << shape->y + shape->r << ")" << "\n";
}

void goltsov::frameset(SharedCircles& shared_circles)
{
  std::vector< std::string > args = read_args();
  check(args.size() == 1);
  std::vector< std::string > set = sets.at(args[0]);
  double min_x = std::numeric_limits< double >::max();
  double max_x = std::numeric_limits< double >::lowest();
  double min_y = std::numeric_limits< double >::max();
  double max_y = std::numeric_limits< double >::lowest();
  for (size_t i = 0; i < set.size(); ++i)
  {
    const Circle* shape = shared_circles.getData() + name_idx_map.at(set[i]);
    min_x = std::min< double >(min_x, shape->x - shape->r);
    max_x = std::max< double >(max_x, shape->x + shape->r);
    min_y = std::min< double >(min_y, shape->y - shape->r);
    max_y = std::max< double >(max_y, shape->y + shape->r);
  }
  std::cout << "(" << min_x << " " << min_y << ") (" << max_x << " " << max_y << ")" << "\n";
}

void goltsov::spawn(SharedCircles&)
{
  std::vector< std::string > args = read_args();
  check(args.size() == 2);
  const std::string& process_name = args[0];
  long long seed = toInteger(args[1]);
  check(isValidName(process_name) && seed >= 0 && !processes.count(process_name));
  int pps1[2];
  if (pipe(pps1) < 0)
  {
    throw std::runtime_error("pipe");
  }
  int pps2[2];
  if (pipe(pps2) < 0)
  {
    close(pps1[0]);
    close(pps1[1]);
    throw std::runtime_error("pipe");
  }
  pid_t pid = fork();
  if (pid < 0)
  {
    close(pps1[0]);
    close(pps1[1]);
    close(pps2[0]);
    close(pps2[1]);
    throw std::runtime_error("pid");
  }
  if (pid == 0)
  {
    close(pps1[1]);
    close(pps2[0]);
    for (auto& p : processes)
    {
      close(p.second.wr);
      close(p.second.rd);
    }
    _exit(runChild(pps1[0], pps2[1], seed));
  }
  close(pps1[0]);
  close(pps2[1]);
  Process process = Process{process_name, pps1[1], pps2[0], static_cast< size_t >(seed), pid, {}};
  processes.insert({process_name, std::move(process)});
}

void goltsov::areasOn(SharedCircles& shared_circles)
{
  std::vector< std::string > args = read_args();
  check(args.size() == 5);
  const std::string& process_name = args[0];
  const std::string& calc_name = args[1];
  const std::string& set_name = args[2];
  long long threads = toInteger(args[3]);
  long long tries = toInteger(args[4]);
  check(isValidName(calc_name) && threads > 0 && tries > 0 && !calculations.count(calc_name));
  std::vector< std::string > set = sets.at(set_name);
  Process& process = processes.at(process_name);
  const size_t count_circles = shared_circles.getCountCircles();
  const size_t count_circles_in_set = set.size();
  const size_t threads_count = threads;
  const size_t tries_count = tries;
  std::vector< size_t > indices;
  for (size_t i = 0; i < count_circles_in_set; ++i)
  {
    indices.push_back(name_idx_map.at(set[i]));
  }
  sendValue(process.wr, count_circles);
  sendValue(process.wr, threads_count);
  sendValue(process.wr, tries_count);
  sendValue(process.wr, count_circles_in_set);
  for (size_t i = 0; i < count_circles_in_set; ++i)
  {
    sendValue(process.wr, indices[i]);
  }
  process.pending.push_back(calc_name);
  calculations.insert({calc_name, Calculation{process_name, false, 0, 0.0}});
}

void goltsov::status(SharedCircles&)
{
  std::vector< std::string > args = read_args();
  check(args.size() == 1);
  const Calculation& calculation = calculations.at(args[0]);
  Process& process = processes.at(calculation.process);
  if (!calculation.done)
  {
    update(process);
  }
  if (calculation.done)
  {
    printResult(process, calculation);
  }
  else
  {
    std::cout << process.name << " <IN PROGRESS>\n";
  }
}

void goltsov::wait(SharedCircles&)
{
  std::vector< std::string > args = read_args();
  check(args.size() == 1);
  const Calculation& calculation = calculations.at(args[0]);
  Process& process = processes.at(calculation.process);
  while (!calculation.done)
  {
    readResult(process);
  }
  printResult(process, calculation);
}

void goltsov::shutdown()
{
  for (auto& p : processes)
  {
    close(p.second.wr);
  }
  for (auto& p : processes)
  {
    waitpid(p.second.pid, nullptr, 0);
    close(p.second.rd);
  }
  processes.clear();
}
