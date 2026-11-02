#ifndef WORKER_HPP
#define WORKER_HPP

#include <cstddef>
#include <map>
#include <string>

#include <sys/types.h>

namespace petrov {
  struct Request {
    size_t calc_id;
    size_t threads;
    size_t tries;
    size_t shape_count;
  };

  struct Response {
    size_t calc_id;
    long long time_ms;
    double area;
  };

  struct Worker {
    pid_t pid;
    int request_fd;
    int response_fd;
  };
}

#endif
