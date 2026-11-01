#ifndef PIPEIO_HPP
#define PIPEIO_HPP
#include <cstddef>

namespace zharov
{
  size_t sendAll(int& err, int wr, const char* b, size_t k);
  size_t recvAll(int& err, int rd, char* b, size_t k);
}

#endif
