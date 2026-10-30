#ifndef STRUCTURES_HPP
#define STRUCTURES_HPP

#include <cstddef>
#include <map>
#include <vector>
#include <string>
#include <sys/types.h>

namespace vasyakin
{
  constexpr size_t MAX_NAME_LEN = 64;
  constexpr size_t BATCH_SIZE = 50;

  struct circle_t
  {
    int r;
    int x, y;
  };

  struct rectangle_t
  {
    int min_x, max_x;
    int min_y, max_y;
  };

  struct ShapeSet
  {
    std::vector< std::string > shape_names;
  };

  enum class CommandType
  {
    CALCULATION,
    SHUTDOWN
  };

  struct CommandHeader
  {
    CommandType type;
    char calc_name[MAX_NAME_LEN];
    size_t shape_count;
    size_t threads;
    size_t tries;
  };

  struct ShapeBatch
  {
    size_t count;
    circle_t shapes[BATCH_SIZE];
  };

  struct Result
  {
    char calc_name[MAX_NAME_LEN];
    double area;
    long long time_ms;
  };

  struct ProcessInfo
  {
    pid_t pid;
    int wr_fd;
    int rd_fd;
    unsigned int seed;
    bool alive;
  };

  enum class CalcStatus
  {
    IN_PROGRESS,
    COMPLETED
  };

  struct Calculation
  {
    std::string process_name;
    CalcStatus status;
    double area;
    long long time_ms;
  };

  struct State
  {
    std::string exe_path;
    std::map< std::string, circle_t > circles;
    std::map< std::string, ShapeSet > sets;
    std::map< std::string, ProcessInfo > processes;
    std::map< std::string, Calculation > calculations;
  };
}

#endif
