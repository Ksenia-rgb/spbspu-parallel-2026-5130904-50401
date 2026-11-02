#ifndef CALCULATION_HPP
#define CALCULATION_HPP

#include "geometry.hpp"

#include <cstddef>
#include <sys/types.h>

#include <deque>
#include <ios>
#include <unordered_map>

namespace chernov {
  struct App;
  enum class ProcessState { Active, Stopping, Stopped };

  struct process_t {
    pid_t pid;
    size_t seed;
    int task_fd;
    int result_fd;
    ProcessState state;
    std::string current_calc_name;
    std::deque< std::string > pending_calcs;
    std::vector< char > buffer{};
  };

  enum class TaskState { Pending, Running, Completed, Cancelled, Failed };

  struct calc_t {
    std::string process_name;
    TaskState state;
    double milliseconds;
    double result;
    size_t id;
    std::vector< ellipse_t > ellipses;
    size_t threads;
    size_t tries;
  };

  struct Task {
    size_t calc_id;
    size_t count;
    size_t threads;
    size_t tries;
  };

  struct Result {
    size_t calc_id;
    double milliseconds;
    double area;
  };

  using processes_t = std::unordered_map< std::string, process_t >;
  using calcs_t = std::unordered_map< std::string, calc_t >;

  size_t recvAll(int &err, int rd, char *b, size_t k);
  size_t sendAll(int &err, int wr, const char *b, size_t k);
  void calculationLoop(int task_fd, int result_fd, size_t seed);
  void startNextCalculation(App &app, process_t &process);
  void receiveAvailableResult(App &, process_t &);

  bool isInside(double x, double y, const ellipse_t &ellipse);
  bool isInsideAny(point_t point, const std::vector< ellipse_t > &ellipses);
  size_t calc(const std::vector< ellipse_t > &ellipses, const frame_t &frame, size_t tests, size_t seed);
  double areaOf(const std::vector< ellipse_t > &ellipses, size_t threads, size_t tries, size_t seed);
}

#endif
