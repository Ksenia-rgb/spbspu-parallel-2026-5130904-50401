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
}
