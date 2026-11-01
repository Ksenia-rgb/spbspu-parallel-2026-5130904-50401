#include "protocol.hpp"
#include "pipeio.hpp"

bool zharov::sendRequest(int wr, const Request& req, const std::vector< Circle >& shapes)
{
  int err = 0;
  const char* const head = reinterpret_cast< const char* >(&req);
  if (sendAll(err, wr, head, sizeof(req)) != sizeof(req))
  {
    return false;
  }
  const char* const body = reinterpret_cast< const char* >(shapes.data());
  const size_t bytes = shapes.size() * sizeof(Circle);
  return sendAll(err, wr, body, bytes) == bytes;
}

bool zharov::recvRequest(int rd, Request& req, std::vector< Circle >& shapes)
{
  int err = 0;
  char* const head = reinterpret_cast< char* >(&req);
  if (recvAll(err, rd, head, sizeof(req)) != sizeof(req))
  {
    return false;
  }
  shapes.resize(req.count);
  char* const body = reinterpret_cast< char* >(shapes.data());
  const size_t bytes = shapes.size() * sizeof(Circle);
  return recvAll(err, rd, body, bytes) == bytes;
}

bool zharov::sendResponse(int wr, const Response& res)
{
  int err = 0;
  const char* const data = reinterpret_cast< const char* >(&res);
  return sendAll(err, wr, data, sizeof(res)) == sizeof(res);
}

bool zharov::recvResponse(int rd, Response& res)
{
  int err = 0;
  char* const data = reinterpret_cast< char* >(&res);
  return recvAll(err, rd, data, sizeof(res)) == sizeof(res);
}
