#ifndef STUDILOVA_PIPE_IO_HPP
#define STUDILOVA_PIPE_IO_HPP

#include <cstddef>

namespace studilova
{
  size_t readAll(int& error, int descriptor, void* data, size_t size);
  size_t writeAll(int& error, int descriptor, const void* data, size_t size);
}

#endif
