#include "interprocess.hpp"

#include <cerrno>
#include <stdexcept>
#include <system_error>
#include <unistd.h>

void lukashevich::sendAll(int descriptor, const void *data, std::size_t size)
{
  const char *current = static_cast< const char * >(data);
  while (size != 0)
  {
    const ssize_t sent = write(descriptor, current, size);
    if (sent < 0 && errno == EINTR)
    {
      continue;
    }
    if (sent <= 0)
    {
      throw std::system_error(errno, std::generic_category(), "write");
    }
    current += sent;
    size -= static_cast< std::size_t >(sent);
  }
}

