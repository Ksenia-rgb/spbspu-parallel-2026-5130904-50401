#ifndef CALCULATION_HPP
#define CALCULATION_HPP

#include "geometry.hpp"

#include <ios>
#include <unordered_map>

namespace chernov {
  struct process_t {
    pid_t pid;
    size_t seed;
    int to_child[2];
    int to_parent[2];
  };

  struct calc_t {
    std::string process_name;
    bool done;
    double milliseconds;
    double result;
  };

  struct Task {
    size_t offset;
    size_t count;
    size_t threads;
    size_t tries;
  };

  using processes_t = std::unordered_map< std::string, process_t >;
  using calcs_t = std::unordered_map< std::string, calc_t >;

  bool read_all(int fd, void *buffer, size_t size);
  bool write_all(int fd, const void *buffer, size_t size);
  void calculationLoop(int task_fd, int result_fd, size_t seed, const void *shared_memory, size_t shared_bytes);

  bool isInside(double x, double y, const ellipse_t &ellipse);
  bool isInsideAny(point_t point, const std::vector< ellipse_t > &ellipses);
  size_t calc(const std::vector< ellipse_t > &ellipses, const frame_t &frame, size_t tests, size_t seed);
  double areaOf(const std::vector< ellipse_t > &ellipses, size_t threads, size_t tries, size_t seed);
}

#endif
