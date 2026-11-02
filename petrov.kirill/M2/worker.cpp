#include "worker.hpp"

#include <cerrno>
#include <chrono>
#include <csignal>
#include <exception>
#include <vector>

#include <poll.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include "area.hpp"
#include "shape.hpp"

namespace {
  const int no_fd = -1;
  const int error_exit_code = 1;
  volatile std::sig_atomic_t stop_requested = 0;

  void requestStop(int)
  {
    stop_requested = 1;
  }

  void setStopHandler()
  {
    struct sigaction action{};
    action.sa_handler = requestStop;
    action.sa_flags = SA_RESTART;
    sigemptyset(&action.sa_mask);
    sigaction(SIGUSR1, &action, nullptr);
  }

  void closeFd(int& fd)
  {
    if (fd != no_fd) {
      close(fd);
      fd = no_fd;
    }
  }

  void workerLoop(int request_fd, int response_fd, size_t seed)
  {
    using Clock = std::chrono::steady_clock;
    using Milliseconds = std::chrono::milliseconds;
    while (stop_requested == 0) {
      petrov::Request request{};
      if (!petrov::readAll(request_fd, &request, sizeof(request))) {
        return;
      }
      std::vector< petrov::Shape > shapes(request.shape_count);
      if (!petrov::readAll(request_fd, shapes.data(), sizeof(petrov::Shape) * shapes.size())) {
        return;
      }
      const Clock::time_point start = Clock::now();
      const double area = petrov::countArea(shapes, request.threads, request.tries, seed);
      const Clock::time_point finish = Clock::now();
      const long long time_ms = std::chrono::duration_cast< Milliseconds >(finish - start).count();
      const petrov::Response response{request.calc_id, time_ms, area};
      if (!petrov::writeAll(response_fd, &response, sizeof(response))) {
        return;
      }
    }
  }
}
