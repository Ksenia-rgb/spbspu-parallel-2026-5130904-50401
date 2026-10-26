#include <unistd.h>
#include <cerrno>
#include "ipc.hpp"

size_t vasyakin::recv(int& err, int fd, char* b, size_t k)
{
  size_t r = 0;
  while (r < k)
  {
    ssize_t n = read(fd, b + r, k - r);
    if (n < 0)
    {
      err = errno;
      break;
    }

    if (n == 0)
    {
      err = 0;
      break;
    }

    r += n;
  }

  return r;
}

size_t vasyakin::send(int& err, int fd, char* b, size_t k)
{
  size_t r = 0;
  while (r < k)
  {
    ssize_t n = write(fd, b + r, k - r);

    if (n < 0)
    {
      err = errno;
      break;
    }

    if (n == 0)
    {
      err = 0;
      break;
    }

    r += static_cast< size_t >(n);
  }

  return r;
}
