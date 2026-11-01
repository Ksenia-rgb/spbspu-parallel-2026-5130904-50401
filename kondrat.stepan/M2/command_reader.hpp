#ifndef COMMAND_READER_HPP
#define COMMAND_READER_HPP

#include <string>
#include "jobs.hpp"

namespace kondrat
{
  class CommandReader
  {
  public:
    CommandReader();

    bool readLine(std::string & line, Jobs & jobs);

  private:
    std::string pending_;
    bool finished_;
  };
}

#endif
