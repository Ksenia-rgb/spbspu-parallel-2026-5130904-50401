#ifndef WORKER_HPP
#define WORKER_HPP
#include <cstddef>
#include <memory>
#include <vector>
#include <sys/types.h>
#include "fd.hpp"
#include "protocol.hpp"
#include "shapes.hpp"

namespace zharov
{
  class Worker
  {
  public:
    Worker(size_t seed, const std::vector< int >& inherited);
    ~Worker();
    Worker(const Worker&) = delete;
    Worker& operator=(const Worker&) = delete;
    bool exchange(const Request& req, const std::vector< Circle >& shapes, Response& res);
    int toChildFd() const noexcept;
    int fromChildFd() const noexcept;

  private:
    pid_t pid_;
    std::unique_ptr< Fd > to_child_;
    std::unique_ptr< Fd > from_child_;
  };

  void childLoop(int rd, int wr, size_t seed);
}

#endif
