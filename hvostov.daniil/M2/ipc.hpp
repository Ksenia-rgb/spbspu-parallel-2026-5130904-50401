#ifndef IPC_HPP
#define IPC_HPP

#include <cstddef>
#include "types.hpp"

namespace hvostov {

  size_t send(int& err, int wr, const char* b, size_t k);
  size_t recv(int& err, int rd, char* b, size_t k);

  struct AreaRequestHeader {
    char calc_name[64];
    int threads;
    size_t tries;
    size_t shape_count;
    size_t seed;
  };

  struct AreaResultHeader {
    char calc_name[64];
    double result;
    double time_ms;
  };

  constexpr size_t MAX_SHAPES = 4096;

  struct ShmLayout {
    AreaRequestHeader req;
    shape_t shapes[MAX_SHAPES];
    AreaResultHeader res;
  };

}

#endif
