#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <cstddef>

namespace kondrat
{
  bool readFull(int descriptor, void * buffer, size_t size);
  void writeFull(int descriptor, const void * buffer, size_t size);
}

#endif
