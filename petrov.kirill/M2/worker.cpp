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

bool petrov::startWorker(size_t seed, const Workers& others, Worker& worker)
{
  int requests[2] = {no_fd, no_fd};
  int responses[2] = {no_fd, no_fd};
  if (pipe(requests) == -1) {
    return false;
  }
  if (pipe(responses) == -1) {
    close(requests[0]);
    close(requests[1]);
    return false;
  }
  setStopHandler();
  const pid_t pid = fork();
  if (pid == -1) {
    close(requests[0]);
    close(requests[1]);
    close(responses[0]);
    close(responses[1]);
    return false;
  }
  if (pid == 0) {
    close(requests[1]);
    close(responses[0]);
    for (Workers::const_iterator it = others.begin(); it != others.end(); ++it) {
      Worker other = it->second;
      closeFd(other.request_fd);
      closeFd(other.response_fd);
    }
    try {
      workerLoop(requests[0], responses[1], seed);
    } catch (const std::exception&) {
      _exit(error_exit_code);
    }
    _exit(0);
  }
  close(requests[0]);
  close(responses[1]);
  worker.pid = pid;
  worker.request_fd = requests[1];
  worker.response_fd = responses[0];
  return true;
}

bool petrov::isRunning(const Worker& worker)
{
  return worker.response_fd != no_fd;
}

bool petrov::acceptsRequests(const Worker& worker)
{
  return worker.request_fd != no_fd;
}

bool petrov::hasResponse(const Worker& worker, int wait_ms)
{
  const nfds_t fd_count = 1;
  pollfd waiting{};
  waiting.fd = worker.response_fd;
  waiting.events = POLLIN;
  return poll(&waiting, fd_count, wait_ms) > 0;
}

void petrov::stopWorker(Worker& worker)
{
  if (isRunning(worker)) {
    kill(worker.pid, SIGUSR1);
  }
  closeFd(worker.request_fd);
}

void petrov::finishWorker(Worker& worker)
{
  closeFd(worker.request_fd);
  closeFd(worker.response_fd);
  waitpid(worker.pid, nullptr, 0);
}

void petrov::stopAllWorkers(Workers& workers)
{
  for (Workers::iterator it = workers.begin(); it != workers.end(); ++it) {
    Worker& worker = it->second;
    if (isRunning(worker)) {
      kill(worker.pid, SIGTERM);
      finishWorker(worker);
    }
  }
}
