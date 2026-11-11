#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <cstddef>
#include <map>
#include <string>
#include <iosfwd>
#include "descriptor.hpp"

namespace studilova
{
  struct process_t
  {
    pid_t pid;
    Descriptor write_descriptor;
    Descriptor read_descriptor;
  };

  struct calculation_t
  {
    std::size_t id;
    std::string process_name;
    bool completed;
    bool success;
    double area;
    double milliseconds;
  };

  using processes_t = std::map< std::string, process_t >;
  using calculations_t = std::map< std::string, calculation_t >;

  void runProcess(int read_descriptor, int write_descriptor, unsigned int seed);
  process_t startProcess(unsigned int seed, const processes_t& processes);
  void receiveResult(process_t& process, const std::string& process_name, calculations_t& calculations);
  int finishProcesses(processes_t& processes, calculations_t& calculations, std::ostream& errors);
}

#endif
