#include "ipc.hpp"
#include <unistd.h>

size_t goltsov::send(int& err, int wr, const char* b, size_t k)
{
  size_t r = 0;
  while (r < k)
  {
    err = write(wr, b + r, k - r);
    if (err < 0)
    {
      break;
    }
    r += err;
  }
  return r;
}

size_t goltsov::recv(int& err, int rd, char* b, size_t k)
{
  size_t r = 0;
  while (r < k)
  {
    err = read(rd, b + r, k - r);
    if (err <= 0)
    {
      break;
    }
    r += err;
  }
  return r;
}
