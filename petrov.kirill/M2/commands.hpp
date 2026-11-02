#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <cstddef>
#include <iosfwd>
#include <map>
#include <string>
#include <vector>

#include "shape.hpp"
#include "worker.hpp"

namespace petrov {
  enum class CalcStatus {
    InProgress,
    Done,
    Canceled
  };

  struct Calc {
    size_t id;
    std::string worker_name;
    CalcStatus status;
    long long time_ms;
    double area;
  };

  using Words = std::vector< std::string >;
  using Shapes = std::map< std::string, Shape >;
  using Sets = std::map< std::string, Words >;
  using Calcs = std::map< std::string, Calc >;

  struct Context {
    Shapes shapes;
    Sets sets;
    Workers workers;
    Calcs calcs;
    size_t next_calc_id = 0;
  };

  using Command = bool (*)(const Words& words, Context& context, std::ostream& out);
  using Commands = std::map< std::string, Command >;

  Commands getCommands();
}

#endif
