#ifndef PROCESS_MANAGER_HPP
#define PROCESS_MANAGER_HPP

#include "process.hpp"
#include "shape_manager.hpp"
#include <iosfwd>
#include <map>
#include <queue>
#include <string>
#include <sys/types.h>
#include <vector>

namespace donkeev {
bool writeAll(int, const void *, size_t);

bool readAll(int, void *, size_t);

struct RawCircleData {
  int r;
  int x;
  int y;
};

struct RawResultData {
  double area;
  double time_ms;
};

void monteCarloWorker(const std::vector<circle_t> &, const rectangle_t &,
                      size_t, size_t, size_t &);

void runChildLoop(int, int, size_t);

struct FlatCircle {
  double cx, cy, r2;
};

struct CalculationInfo {
  std::string process_name;
  bool completed;
  double area;
  double time_ms;
};

class ProcessManager {
public:
  ProcessManager() = default;
  ~ProcessManager();

  ProcessManager(const ProcessManager &) = delete;
  ProcessManager &operator=(const ProcessManager &) = delete;

  void addProcess(const std::string &, const Process &);
  bool hasProcess(const std::string &) const;
  bool hasCalculation(const std::string &) const;
  std::vector<Process> getAllProcesses() const;

  void startCalculation(const std::string &, const std::string &,
                        const std::string &, const ShapeManager &, size_t,
                        size_t);

  void printStatus(const std::string &, std::ostream &);
  void waitCalculation(const std::string &, std::ostream &);

private:
  std::map<std::string, Process> processes_;
  std::map<std::string, std::queue<std::string>> pending_calcs_;
  std::map<std::string, CalculationInfo> calculations_;

  void pollProcessResults(const std::string &);
  void readOneResultBlocking(const std::string &);
};

Process spawnProcess(size_t, const std::vector<Process> &);
void closeProcess(Process &);
} // namespace donkeev

#endif
