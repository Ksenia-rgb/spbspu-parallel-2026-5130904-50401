#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <map>
#include <string>
#include <sys/types.h>

namespace zhuravleva
{
  struct Process
  {
    pid_t pid;
    int write_fd;
    int read_fd;
    unsigned int seed;
  };

  struct ProcessStorage
  {
    std::map< std::string, Process > processes;
  };

  bool spawnProcess(ProcessStorage& storage, const std::string& name, unsigned int seed);
  bool killProcess(ProcessStorage& storage, const std::string& name);
  void shutdownProcesses(ProcessStorage& storage);
}

#endif
