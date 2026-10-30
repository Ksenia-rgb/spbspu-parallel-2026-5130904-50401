#ifndef IPC_HPP
#define IPC_HPP

#include <cstddef>

namespace vasyakin
{
  size_t recv(int& err, int rd, char* b, size_t k);
  size_t send(int& err, int fd, char* b, size_t k);
}

#endif
