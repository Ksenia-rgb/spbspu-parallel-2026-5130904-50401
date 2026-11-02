#include "calculations.hpp"

#include "geometry.hpp"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <limits>
#include <functional>
#include <future>
#include <random>
#include <unistd.h>

size_t chernov::recvAll(int &err, int rd, char *b, size_t k)
{
  err = 0;
  size_t received = 0;
  while (received < k) {
    const size_t chunk = std::min(k - received, static_cast< size_t >(std::numeric_limits< ssize_t >::max()));
    const ssize_t n = read(rd, b + received, chunk);
    if (n == -1) {
      if (errno == EINTR) {
        continue;
      }
      err = errno;
      break;
    }
    if (n == 0) {
      break;
    }
    received += static_cast< size_t >(n);
  }
  return received;
}

size_t chernov::sendAll(int &err, int wr, const char *b, size_t k)
{
  err = 0;
  size_t sent = 0;
  while (sent < k) {
    const size_t chunk = std::min(k - sent, static_cast< size_t >(std::numeric_limits< ssize_t >::max()));
    const ssize_t n = write(wr, b + sent, chunk);
    if (n == -1) {
      if (errno == EINTR) {
        continue;
      }
      err = errno;
      break;
    }
    if (n == 0) {
      err = EIO;
      break;
    }
    sent += static_cast< size_t >(n);
  }
  return sent;
}

[[noreturn]] void chernov::calculationLoop(int task_fd, int result_fd, size_t seed)
{
  int exit_code = 0;
  try {
    struct sigaction action{};
    action.sa_handler = SIG_IGN;
    if (sigemptyset(&action.sa_mask) == -1 || sigaction(SIGPIPE, &action, nullptr) == -1) {
      close(task_fd);
      close(result_fd);
      _exit(1);
    }

    std::vector< ellipse_t > ellipses;
    while (true) {
      Task task{};
      int err = 0;
      const size_t received = recvAll(err, task_fd, reinterpret_cast< char * >(&task), sizeof(task));
      if (received == 0 && err == 0) {
      }
      if (err != 0 || received != sizeof(task) || task.count == 0 || task.threads == 0 || task.tries == 0) {
        exit_code = 1;
        break;
      }
      if (task.count > ellipses.max_size() || task.count > std::numeric_limits< size_t >::max() / sizeof(ellipse_t)) {
        exit_code = 1;
        break;
      }

      ellipses.resize(task.count);
      const size_t bytes = task.count * sizeof(ellipse_t);
      const size_t body_received = recvAll(err, task_fd, reinterpret_cast< char * >(ellipses.data()), bytes);
      if (err != 0 || body_received != bytes) {
        exit_code = 1;
        break;
      }

      const auto start = std::chrono::steady_clock::now();
      const double area = areaOf(ellipses, task.threads, task.tries, seed);
      const auto end = std::chrono::steady_clock::now();
      const double milliseconds = std::chrono::duration< double, std::milli >(end - start).count();

      Result result{};
      result.calc_id = task.calc_id;
      result.milliseconds = milliseconds;
      result.area = area;
      const size_t sent = sendAll(err, result_fd, reinterpret_cast< const char * >(&result), sizeof(result));
      if (err != 0 || sent != sizeof(result)) {
        exit_code = 1;
        break;
      }
    }
  } catch (...) {
    exit_code = 1;
  }

  close(task_fd);
  close(result_fd);
  _exit(exit_code);
}

bool chernov::isInside(const double x, const double y, const ellipse_t &ellipse)
{
  const double dx = x - ellipse.center.x;
  const double dy = y - ellipse.center.y;
  const double a = ellipse.horizontal_radius;
  const double b = ellipse.vertical_radius;
  return dx * dx / (a * a) + dy * dy / (b * b) <= 1;
}

bool chernov::isInsideAny(point_t point, const std::vector< ellipse_t > &ellipses)
{
  const auto bound_is_inside = std::bind(isInside, point.x, point.y, std::placeholders::_1);
  return std::any_of(ellipses.begin(), ellipses.end(), bound_is_inside);
}

size_t chernov::calc(
    const std::vector< ellipse_t > &ellipses, const frame_t &frame, const size_t tests, const size_t seed)
{
  std::default_random_engine gen(seed);
  std::uniform_real_distribution<> dis_x(frame.left_bottom.x, frame.right_top.x);
  std::uniform_real_distribution<> dis_y(frame.left_bottom.y, frame.right_top.y);

  size_t count = 0;
  for (size_t i = 0; i < tests; ++i) {
    const double x = dis_x(gen);
    const double y = dis_y(gen);

    if (isInsideAny(point_t{x, y}, ellipses)) {
      ++count;
    }
  }
  return count;
}

double chernov::areaOf(const std::vector< ellipse_t > &ellipses, const size_t threads, const size_t tries, size_t seed)
{
  frame_t frame = getFrame(ellipses);

  std::vector< std::future< size_t > > futures;
  const size_t batch = tries / threads;
  futures.emplace_back(std::async(std::launch::async, calc, std::cref(ellipses), frame, batch + tries % threads, seed));
  for (size_t i = 1; i < threads; ++i) {
    futures.emplace_back(std::async(std::launch::async, calc, std::cref(ellipses), frame, batch, seed + i));
  }

  size_t count = 0;
  for (size_t i = 0; i < threads; ++i) {
    count += futures[i].get();
  }

  const double frame_area = (frame.right_top.x - frame.left_bottom.x) * (frame.right_top.y - frame.left_bottom.y);
  const double area = frame_area * count / tries;
  return area;
}
