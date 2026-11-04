#include "pipe.hpp"
#include <cerrno>
#include <unistd.h>

std::size_t studilova::readAll(int& error, int descriptor,
    void* data, std::size_t size)
{
  char* bytes = static_cast< char* >(data);
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

std::size_t studilova::writeAll(int& error, int descriptor,
    const void* data, std::size_t size)
{
  const char* bytes = static_cast< const char* >(data);
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
