#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <cstddef>

namespace kondrat
{
  enum class RequestKind
  {
    Area,
    Stop
  };

  struct Request
  {
    RequestKind kind;
    unsigned long long calculation_id;
    long long threads;
    long long tries;
    size_t shape_count;
  };

  struct Response
  {
    unsigned long long calculation_id;
    double time_ms;
    double area;
  };
}

#endif
