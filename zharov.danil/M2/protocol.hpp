#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP
#include <cstddef>
#include <vector>
#include "shapes.hpp"

namespace zharov
{
  enum class RequestKind
  {
    CALC,
    STATUS,
    WAIT,
    STOP
  };

  enum class CalcState
  {
    PROGRESS,
    DONE
  };

  struct Request
  {
    RequestKind kind;
    size_t calc_id;
    size_t threads;
    size_t tries;
    size_t count;
  };

  struct Response
  {
    size_t calc_id;
    CalcState state;
    double ms;
    double area;
  };

  bool sendRequest(int wr, const Request& req, const std::vector< Circle >& shapes);
  bool recvRequest(int rd, Request& req, std::vector< Circle >& shapes);
  bool sendResponse(int wr, const Response& res);
  bool recvResponse(int rd, Response& res);
}

#endif
