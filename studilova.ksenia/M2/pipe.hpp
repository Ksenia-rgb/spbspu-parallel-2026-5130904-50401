#ifndef PIPE_HPP
#define PIPE_HPP

#include <cstddef>
#include <vector>
#include "geometry.hpp"

namespace studilova
{
  struct request_t
  {
    size_t calculation_id;
    size_t threads;
    size_t tries;
    size_t circle_count;
  };

  struct response_t
  {
    size_t calculation_id;
    bool success;
    double area;
    double milliseconds;
  };

  size_t readAll(int& error, int descriptor, void* data, size_t size);
  size_t writeAll(int& error, int descriptor, const void* data, size_t size);
  void sendRequest(int descriptor, const request_t& request, const std::vector< circle_t >& circles);
}

#endif
