#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <cstddef>
#include <map>
#include <string>
#include <sys/types.h>
#include "descriptor.hpp"


namespace studilova
{
  struct process_t
  {
    pid_t pid;
    Descriptor writeDescriptor;
    Descriptor readDescriptor;
  };

  struct calculation_t
  {
    std::size_t id;
    std::string processName;
    bool completed;
    bool success;
    double area;
    double milliseconds;
  };

  using processes_t = std::map< std::string, process_t >;
  using calculations_t = std::map< std::string, calculation_t >;

  void runProcess(int readDescriptor, int writeDescriptor, unsigned int seed);
  process_t startProcess(unsigned int seed, const processes_t& processes);
}

#endif
