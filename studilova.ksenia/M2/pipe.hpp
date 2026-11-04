#ifndef PIPE_HPP
#define PIPE_HPP

#include <cstddef>

namespace studilova
{
  struct request_t
  {
    size_t calculationId;
    size_t threads;
    size_t tries;
    size_t circleCount;
  };

  struct response_t
  {
    size_t calculationId;
    bool success;
    double area;
    double milliseconds;
  };

  size_t readAll(int& error, int descriptor, void* data, size_t size);
  size_t writeAll(int& error, int descriptor, const void* data, size_t size);
}

#endif
