#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <string>
#include <vector>
#include <map>
#include "shape_manager.hpp"

namespace donkeev
{
  using ValidatorFunc = bool (*)(const std::vector<std::string>&);
  using ExecutorFunc = void (*)(ShapeManager&, const std::vector<std::string>&);

  struct Command
  {
    ValidatorFunc validate;
    ExecutorFunc execute;
  };

  bool validateCircle(const std::vector<std::string>& tokens);
  bool validateSet(const std::vector<std::string>& tokens);
  bool validateTwoTokens(const std::vector<std::string>& tokens);
  bool validateSpawn(const std::vector<std::string>& tokens);
  bool validateAreaOn(const std::vector<std::string>& tokens);

  void executeCircle(ShapeManager& manager, const std::vector<std::string>& tokens);
  void executeSet(ShapeManager& manager, const std::vector<std::string>& tokens);
  void executeShow(ShapeManager& manager, const std::vector<std::string>& tokens);
  void executeShowSet(ShapeManager& manager, const std::vector<std::string>& tokens);
  void executeFrame(ShapeManager& manager, const std::vector<std::string>& tokens);
  void executeFrameSet(ShapeManager& manager, const std::vector<std::string>& tokens);

  void executeSpawn(ShapeManager& manager, const std::vector<std::string>& tokens);
  void executeAreaOn(ShapeManager& manager, const std::vector<std::string>& tokens);
  void executeStatus(ShapeManager& manager, const std::vector<std::string>& tokens);
  void executeWait(ShapeManager& manager, const std::vector<std::string>& tokens);

}

#endif
