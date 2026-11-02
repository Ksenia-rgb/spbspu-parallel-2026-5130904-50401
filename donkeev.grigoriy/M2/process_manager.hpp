#ifndef PROCESS_MANAGER_HPP
#define PROCESS_MANAGER_HPP

#include <string>
#include <map>
#include <vector>
#include <queue>
#include <iosfwd>
#include <sys/types.h>
#include "shape_manager.hpp"
#include "process.hpp"

namespace donkeev
{
  struct CalculationInfo
  {
    std::string process_name_;
    bool completed_;
    double area_;
    double time_ms_;
  };

  class ProcessManager
  {
  public:
    ProcessManager() = default;
    ~ProcessManager();

    ProcessManager(const ProcessManager&) = delete;
    ProcessManager& operator=(const ProcessManager&) = delete;

    void addProcess(const std::string& name, const Process& proc);
    bool hasProcess(const std::string& name) const;
    bool hasCalculation(const std::string& calcName) const;
    std::vector< Process > getAllProcesses() const;

    void startCalculation(const std::string& processName, const std::string& calcName, const std::string& setName,
        const ShapeManager& shapeManager, size_t threads, size_t tries);

    void printStatus(const std::string& calcName, std::ostream& out);
    void waitCalculation(const std::string& calcName, std::ostream& out);

  private:
    std::map< std::string, Process > processes_;
    std::map< std::string, std::queue< std::string > > pending_calcs_;
    std::map< std::string, CalculationInfo > calculations_;

    void pollProcessResults(const std::string& processName);
    void readOneResultBlocking(const std::string& processName);
  };

  Process spawnProcess(size_t seed, const std::vector< Process >& existing_processes);
  void closeProcess(Process& proc);
}

#endif
