#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <sys/types.h>

namespace donkeev
{
  struct Process
  {
    pid_t pid_;
    int pipe_to_child_[2];
    int pipe_from_child_[2];
  };
}

#endif
