#include "pipe.hpp"
#include <cerrno>
#include <unistd.h>
#include <cstring>
#include <stdexcept>

std::size_t studilova::readAll(int& error, int descriptor, void* data, std::size_t size)
{
  char* const bytes = static_cast< char* >(data);
  size_t received = 0;
  error = 0;

  while (received < size)
  {
    const auto result = read(descriptor, bytes + received, size - received);

    if (result == -1)
    {
      error = errno;
      break;
    }

    if (result == 0)
    {
      break;
    }
    received += static_cast< size_t >(result);
  }
  return received;
}

std::size_t studilova::writeAll(int& error, int descriptor, const void* data, std::size_t size)
{
  const char* const bytes = static_cast< const char* >(data);
  size_t sent = 0;
  error = 0;

  while (sent < size)
  {
    const auto result = write(descriptor, bytes + sent, size - sent);

    if (result == -1)
    {
      error = errno;
      break;
    }

    if (result == 0)
    {
      break;
    }
    sent += static_cast< std::size_t >(result);
  }
  return sent;
}

void studilova::sendRequest(int descriptor, const request_t& request, const std::vector< circle_t >& circles)
{
  if (request.circle_count != circles.size())
  {
    throw std::invalid_argument("Incorrect circle count");
  }

  int error = 0;

  const auto sent = writeAll(error, descriptor, &request, sizeof(request));

  if (error != 0)
  {
    throw std::runtime_error(std::strerror(error));
  }

  if (sent != sizeof(request))
  {
    throw std::runtime_error("Incomplete request");
  }

  for (const circle_t& circle : circles)
  {
    const auto circleBytes = writeAll(error, descriptor, &circle, sizeof(circle));

    if (error != 0)
    {
      throw std::runtime_error(std::strerror(error));
    }

    if (circleBytes != sizeof(circle))
    {
      throw std::runtime_error("Incomplete circle");
    }
  }
}
