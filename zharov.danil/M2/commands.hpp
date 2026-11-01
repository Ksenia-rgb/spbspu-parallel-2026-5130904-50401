#ifndef COMMANDS_HPP
#define COMMANDS_HPP
#include <cstddef>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>
#include "shapes.hpp"
#include "worker.hpp"

namespace zharov
{
  using Args = std::vector< std::string >;

  struct Calc
  {
    std::string process;
    size_t id;
    bool done;
    double ms;
    double area;
  };

  struct Context
  {
    std::unordered_map< std::string, Circle > circles;
    std::unordered_map< std::string, std::vector< Circle > > sets;
    std::unordered_map< std::string, std::unique_ptr< Worker > > workers;
    std::unordered_map< std::string, Calc > calcs;
  };

  Args splitLine(const std::string& line);

  void createCircle(std::ostream& out, const Args& args, Context& ctx);
  void createSet(std::ostream& out, const Args& args, Context& ctx);
  void showShape(std::ostream& out, const Args& args, const Context& ctx);
  void showSet(std::ostream& out, const Args& args, const Context& ctx);
  void showFrame(std::ostream& out, const Args& args, const Context& ctx);
  void showFrameSet(std::ostream& out, const Args& args, const Context& ctx);
  void spawnProcess(std::ostream& out, const Args& args, Context& ctx);
  void startArea(std::ostream& out, const Args& args, Context& ctx);
  void showStatus(std::ostream& out, const Args& args, Context& ctx);
  void waitCalc(std::ostream& out, const Args& args, Context& ctx);
}

#endif
