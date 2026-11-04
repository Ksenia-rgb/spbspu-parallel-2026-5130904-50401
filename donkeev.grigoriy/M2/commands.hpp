#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <string>
#include <vector>
#include "shape_manager.hpp"
#include "process_manager.hpp"

namespace donkeev
{
  using ValidatorFunc = bool (*)(const std::vector< std::string >&);
  using ExecutorFunc = void (*)(ShapeManager&, ProcessManager&, const std::vector< std::string >&);

  struct Command
  {
    ValidatorFunc validate;
    ExecutorFunc execute;
  };

  bool validateCircle(const std::vector< std::string >&);
  bool validateSet(const std::vector< std::string >&);
  bool validateTwoTokens(const std::vector< std::string >&);
  bool validateSpawn(const std::vector< std::string >&);
  bool validateAreaOn(const std::vector< std::string >&);

  void executeCircle(ShapeManager&, ProcessManager&, const std::vector< std::string >&);
  void executeSet(ShapeManager&, ProcessManager&, const std::vector< std::string >&);
  void executeShow(ShapeManager&, ProcessManager&, const std::vector< std::string >&);
  void executeShowSet(ShapeManager&, ProcessManager&, const std::vector< std::string >&);
  void executeFrame(ShapeManager&, ProcessManager&, const std::vector< std::string >&);
  void executeFrameSet(ShapeManager&, ProcessManager&, const std::vector< std::string >&);

  void executeSpawn(ShapeManager&, ProcessManager&, const std::vector< std::string >&);
  void executeAreaOn(ShapeManager&, ProcessManager&, const std::vector< std::string >&);
  void executeStatus(ShapeManager&, ProcessManager&, const std::vector< std::string >&);
  void executeWait(ShapeManager&, ProcessManager&, const std::vector< std::string >&);
}

#endif
