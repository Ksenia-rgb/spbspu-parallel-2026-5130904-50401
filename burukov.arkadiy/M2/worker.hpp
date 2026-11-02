#ifndef WORKER_HPP
#define WORKER_HPP

namespace burukov
{
  void runWorkerLoop(int read_fd, int write_fd, long long seed);
}

#endif
