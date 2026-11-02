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

  using Workers = std::map< std::string, Worker >;

  bool startWorker(size_t seed, const Workers& others, Worker& worker);
  bool isRunning(const Worker& worker);
  bool acceptsRequests(const Worker& worker);
  bool hasResponse(const Worker& worker, int wait_ms);
  void stopWorker(Worker& worker);
  void finishWorker(Worker& worker);
  void stopAllWorkers(Workers& workers);
  bool writeAll(int fd, const void* data, size_t size);
  bool readAll(int fd, void* data, size_t size);
}

#endif
