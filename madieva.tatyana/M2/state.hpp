#ifndef STATE_HPP
#define STATE_HPP

#include <map>
#include <vector>
#include <string>
#include "circle.hpp"

namespace madieva
{
  struct Process
  {
    pid_t pid = -1;
    FileDescriptor cmd_wr;
    FileDescriptor res_rd;
  };
  
  struct state_t
  {
    std::map< std::string, Circle > circles;
    std::map< std::string, std::vector< const Circle * > > sets;
    std::map< std::string, Process > processes;
  };
}

#endif
