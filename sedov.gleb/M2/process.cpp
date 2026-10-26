#include <unistd.h>

#include "process.hpp"

namespace {
  size_t send(int & err, int wr, const char * msg, size_t k)
  {
    size_t r = 0;
    while (r < k) {
      err = write(wr, msg + r, k - r);
      if (err <= 0) {
        break;
      }
      r += static_cast< size_t >(err);
    }
    return r;
  }

  size_t recv(int & err, int rd, char * buf, size_t k)
  {
    size_t r = 0;
    while (r < k) {
      err = read(rd, buf + r, k - r);
      if (err <= 0) {
        break;
      }
      r += static_cast< size_t >(err);
    }
    return r;
  }
}
