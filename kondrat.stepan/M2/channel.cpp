#include "channel.hpp"
#include <cerrno>
#include <stdexcept>
#include <system_error>
#include <unistd.h>

bool kondrat::readFull(int descriptor, void * buffer, size_t size)
{
  char * data = static_cast< char * >(buffer);
  size_t received = 0;

  while (received < size)
  {
    const ssize_t count = read(descriptor, data + received, size - received);

    if (count == -1)
    {
      if (errno == EINTR)
      {
        continue;
      }

      throw std::system_error(errno, std::generic_category(), "Cannot read pipe");
    }

    if (count == 0)
    {
      if (received != 0)
      {
        throw std::runtime_error("Incomplete pipe message");
      }

      return false;
    }

    received += static_cast< size_t >(count);
  }

  return true;
}

void kondrat::writeFull(int descriptor, const void * buffer, size_t size)
{
  const char * data = static_cast< const char * >(buffer);
  size_t sent = 0;

  while (sent < size)
  {
    const ssize_t count = write(descriptor, data + sent, size - sent);

    if (count == -1)
    {
      if (errno == EINTR)
      {
        continue;
      }

      throw std::system_error(errno, std::generic_category(), "Cannot write pipe");
    }

    if (count == 0)
    {
      throw std::runtime_error("Cannot finish pipe write");
    }

    sent += static_cast< size_t >(count);
  }
}
