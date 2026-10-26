#include "commands.hpp"
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <deque>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include "child.hpp"
#include "ipc.hpp"

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

    bool is_valid_name(const std::string& name)
    {
      return !name.empty() && std::isalpha(static_cast< unsigned char >(name[0]));
    }

    void read_result(Process& process)
    {
      size_t ms;
      recv_value(process.rd, ms);
      double res;
      recv_value(process.rd, res);
      Calculation& calculation = calculations.at(process.pending.front());
      calculation.ms = ms;
      calculation.area = res;
      calculation.done = true;
      process.pending.pop_front();
    }

    bool is_ready(const Process& process)
    {
      pollfd pfd{process.rd, POLLIN, 0};
      return poll(&pfd, 1, 0) > 0;
    }

    void update(Process& process)
    {
      while (!process.pending.empty() && is_ready(process))
      {
        read_result(process);
      }
    }

    void print_result(const Process& process, const Calculation& calculation)
    {
      printf("%s %zu %.3f\n", process.name.c_str(), calculation.ms, calculation.area);
    }
  }
}

void goltsov::circle(SharedCircles& shared_circles)
{
  std::string name;
  double r;
  double x, y;
  std::cin >> name >> r >> x >> y;
  if (!std::cin || !is_valid_name(name) || r <= 0 || name_idx_map.count(name))
  {
    throw std::runtime_error("<INVALID COMMAND>");
  }
  size_t idx = shared_circles.get_count_circles();
  shared_circles.new_circle(r, x, y);
  name_idx_map.insert({name, idx});
}

void goltsov::set(SharedCircles&)
{
  std::string name;
  long long set_size;
  std::cin >> name >> set_size;
  if (!std::cin || !is_valid_name(name) || set_size <= 0 || sets.count(name))
  {
    throw std::runtime_error("<INVALID COMMAND>");
  }
  std::vector< std::string > shapes;
  for (long long i = 0; i < set_size; ++i)
  {
    std::string shapei;
    std::cin >> shapei;
    if (!std::cin || !name_idx_map.count(shapei))
    {
      throw std::runtime_error("<INVALID COMMAND>");
    }
    shapes.push_back(shapei);
  }
  sets[name] = shapes;
}

void goltsov::show(SharedCircles& shared_circles)
{
  std::string shape_name;
  std::cin >> shape_name;
  if (!std::cin)
  {
    throw std::runtime_error("<INVALID COMMAND>");
  }
  Circle* shape = shared_circles.get_data() + name_idx_map.at(shape_name);
  std::cout << *shape << "\n";
}

void goltsov::showset(SharedCircles& shared_circles)
{
  std::string set_name;
  std::cin >> set_name;
  if (!std::cin)
  {
    throw std::runtime_error("<INVALID COMMAND>");
  }
  std::vector< std::string > set = sets.at(set_name);
  for (size_t i = 0; i < set.size(); ++i)
  {
    std::cout << *(shared_circles.get_data() + name_idx_map.at(set[i])) << "\n";
  }
}

void goltsov::frame(SharedCircles& shared_circles)
{
  std::string shape_name;
  std::cin >> shape_name;
  if (!std::cin)
  {
    throw std::runtime_error("<INVALID COMMAND>");
  }
  Circle* shape = shared_circles.get_data() + name_idx_map.at(shape_name);
  std::cout << "(" << shape->x - shape->r << " " << shape->y - shape->r << ") (" << shape->x + shape->r << " "
            << shape->y + shape->r << ")" << "\n";
}

void goltsov::frameset(SharedCircles& shared_circles)
{
  std::string set_name;
  std::cin >> set_name;
  if (!std::cin)
  {
    throw std::runtime_error("<INVALID COMMAND>");
  }
  std::vector< std::string > set = sets.at(set_name);
  double min_x = std::numeric_limits< double >::max();
  double max_x = std::numeric_limits< double >::lowest();
  double min_y = std::numeric_limits< double >::max();
  double max_y = std::numeric_limits< double >::lowest();
  for (size_t i = 0; i < set.size(); ++i)
  {
    Circle* shape = shared_circles.get_data() + name_idx_map.at(set[i]);
    min_x = std::min< double >(min_x, shape->x - shape->r);
    max_x = std::max< double >(max_x, shape->x + shape->r);
    min_y = std::min< double >(min_y, shape->y - shape->r);
    max_y = std::max< double >(max_y, shape->y + shape->r);
  }
  std::cout << "(" << min_x << " " << min_y << ") (" << max_x << " " << max_y << ")" << "\n";
}

void goltsov::spawn(SharedCircles&)
{
  std::string process_name;
  long long seed;
  std::cin >> process_name >> seed;
  if (!std::cin || !is_valid_name(process_name) || seed < 0 || processes.count(process_name))
  {
    throw std::runtime_error("<INVALID COMMAND>");
  }
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
    _exit(run_child(pps1[0], pps2[1], seed));
  }
  close(pps1[0]);
  close(pps2[1]);
  Process process = Process{process_name, pps1[1], pps2[0], static_cast< size_t >(seed), pid, {}};
  processes.insert({process_name, std::move(process)});
}

void goltsov::areas_on(SharedCircles& shared_circles)
{
  std::string process_name, calc_name, set_name;
  long long threads, tries;
  std::cin >> process_name >> calc_name >> set_name >> threads >> tries;
  if (!std::cin || !is_valid_name(calc_name) || threads <= 0 || tries <= 0 || calculations.count(calc_name))
  {
    throw std::runtime_error("<INVALID COMMAND>");
  }
  std::vector< std::string > set = sets.at(set_name);
  Process& process = processes.at(process_name);
  size_t count_circles = shared_circles.get_count_circles();
  size_t count_circles_in_set = set.size();
  size_t threads_count = threads;
  size_t tries_count = tries;
  std::vector< size_t > indices;
  for (size_t i = 0; i < count_circles_in_set; ++i)
  {
    indices.push_back(name_idx_map.at(set[i]));
  }
  send_value(process.wr, count_circles);
  send_value(process.wr, threads_count);
  send_value(process.wr, tries_count);
  send_value(process.wr, count_circles_in_set);
  for (size_t i = 0; i < count_circles_in_set; ++i)
  {
    send_value(process.wr, indices[i]);
  }
  process.pending.push_back(calc_name);
  calculations.insert({calc_name, Calculation{process_name, false, 0, 0.0}});
}

void goltsov::status(SharedCircles&)
{
  std::string calc_name;
  std::cin >> calc_name;
  if (!std::cin)
  {
    throw std::runtime_error("<INVALID COMMAND>");
  }
  Calculation& calculation = calculations.at(calc_name);
  Process& process = processes.at(calculation.process);
  if (!calculation.done)
  {
    update(process);
  }
  if (calculation.done)
  {
    print_result(process, calculation);
  }
  else
  {
    std::cout << process.name << " <IN PROGRESS>\n";
  }
}

void goltsov::wait(SharedCircles&)
{
  std::string calc_name;
  std::cin >> calc_name;
  if (!std::cin)
  {
    throw std::runtime_error("<INVALID COMMAND>");
  }
  Calculation& calculation = calculations.at(calc_name);
  Process& process = processes.at(calculation.process);
  while (!calculation.done)
  {
    read_result(process);
  }
  print_result(process, calculation);
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
