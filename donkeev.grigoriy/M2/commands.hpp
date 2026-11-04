#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "process_manager.hpp"
#include "shape_manager.hpp"
#include <string>
#include <vector>

namespace donkeev {
using ValidatorFunc = bool (*)(const std::vector<std::string> &);
using ExecutorFunc = void (*)(ShapeManager &, ProcessManager &,
                              const std::vector<std::string> &);

struct Command {
  ValidatorFunc validate;
  ExecutorFunc execute;
};

bool validateCircle(const std::vector<std::string> &tokens);
bool validateSet(const std::vector<std::string> &tokens);
bool validateTwoTokens(const std::vector<std::string> &);
bool validateSpawn(const std::vector<std::string> &tokens);
bool validateAreaOn(const std::vector<std::string> &tokens);

void executeCircle(ShapeManager &manager, ProcessManager &,
                   const std::vector<std::string> &tokens);
void executeSet(ShapeManager &manager, ProcessManager &,
                const std::vector<std::string> &tokens);
void executeShow(ShapeManager &manager, ProcessManager &,
                 const std::vector<std::string> &tokens);
void executeShowSet(ShapeManager &manager, ProcessManager &,
                    const std::vector<std::string> &tokens);
void executeFrame(ShapeManager &manager, ProcessManager &,
                  const std::vector<std::string> &tokens);
void executeFrameSet(ShapeManager &manager, ProcessManager &,
                     const std::vector<std::string> &tokens);

void executeSpawn(ShapeManager &, ProcessManager &procManager,
                  const std::vector<std::string> &tokens);
void executeAreaOn(ShapeManager &manager, ProcessManager &procManager,
                   const std::vector<std::string> &tokens);
void executeStatus(ShapeManager &, ProcessManager &procManager,
                   const std::vector<std::string> &tokens);
void executeWait(ShapeManager &, ProcessManager &procManager,
                 const std::vector<std::string> &tokens);
} // namespace donkeev

#endif
