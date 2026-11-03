#include "calculations.hpp"

#include "commands.hpp"

#include "geometry.hpp"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cmath>
#include <cstring>
#include <limits>
#include <future>
#include <random>
#include <unistd.h>
#include <sys/wait.h>

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

void chernov::calculationLoop(int task_fd, int result_fd, size_t seed)
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
        break;
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

void chernov::startNextCalculation(App &app, process_t &process)
{
  if (process.state != ProcessState::Active || process.current_calc_name != "" || process.pending_calcs.empty()) {
    return;
  }
  std::string calc_name = process.pending_calcs.front();
  calc_t &calc = app.calcs.at(calc_name);

  Task task{calc.id, calc.ellipses.size(), calc.threads, calc.tries};
  if (task.count > calc.ellipses.max_size() || task.count > std::numeric_limits< size_t >::max() / sizeof(ellipse_t)) {
    throw std::runtime_error("overload");
  }

  process.current_calc_name = calc_name;
  int err;
  const size_t sent = sendAll(err, process.task_fd, reinterpret_cast< const char * >(&task), sizeof(task));
  bool failed = err != 0 || sent != sizeof(task);
  if (!failed) {
    const size_t bytes = task.count * sizeof(ellipse_t);
    const size_t body_sent
        = sendAll(err, process.task_fd, reinterpret_cast< const char * >(calc.ellipses.data()), bytes);
    failed = err != 0 || body_sent != bytes;
  }
  if (failed) {
    if (process.task_fd >= 0) {
      close(process.task_fd);
    }
    process.task_fd = -1;
    process.state = ProcessState::Stopping;
    calc.state = TaskState::Failed;
    for (std::string pending_calc_name : process.pending_calcs) {
      app.calcs.at(pending_calc_name).state = TaskState::Failed;
    }
    process.pending_calcs.clear();
    process.current_calc_name.clear();
    return;
  }

  calc.state = TaskState::Running;
  process.pending_calcs.pop_front();
}

void chernov::receiveAvailableResult(App &app, process_t &process)
{
  if (process.result_fd < 0) {
    return;
  }

  auto close_channels = [&]()
  {
    if (process.task_fd >= 0) {
      close(process.task_fd);
      process.task_fd = -1;
    }
    close(process.result_fd);
    process.result_fd = -1;
    process.state = ProcessState::Stopping;
    if (!process.current_calc_name.empty()) {
      calc_t &calc = app.calcs.at(process.current_calc_name);
      if (calc.state == TaskState::Running) {
        calc.state = TaskState::Failed;
      }
    }
    for (const std::string &name : process.pending_calcs) {
      calc_t &calc = app.calcs.at(name);
      if (calc.state == TaskState::Pending || calc.state == TaskState::Running) {
        calc.state = TaskState::Failed;
      }
    }
    process.pending_calcs.clear();
    process.current_calc_name.clear();
    process.buffer.clear();
  };

  const size_t k = sizeof(Result);
  if (process.buffer.size() > k) {
    close_channels();
    throw std::runtime_error("invalid result buffer size");
  }

  process.buffer.reserve(k);
  while (process.buffer.size() < k) {
    char buffer[sizeof(Result)];
    const ssize_t n = read(process.result_fd, buffer, k - process.buffer.size());
    if (n > 0) {
      process.buffer.insert(process.buffer.end(), buffer, buffer + n);
    } else if (n == 0) {
      const bool incomplete = !process.buffer.empty() || !process.current_calc_name.empty();
      close_channels();
      if (incomplete) {
        throw std::runtime_error("result pipe closed before result received");
      }
      return;
    } else if (errno == EINTR) {
      continue;
    } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return;
    } else {
      close_channels();
      throw std::runtime_error("result read failed");
    }
  }

  Result result{};
  std::memcpy(&result, process.buffer.data(), sizeof(result));
  if (process.current_calc_name.empty()) {
    close_channels();
    throw std::runtime_error("unexpected result");
  }
  calc_t &calc = app.calcs.at(process.current_calc_name);
  if (calc.state != TaskState::Running || calc.id != result.calc_id) {
    close_channels();
    throw std::runtime_error("result does not match current calculation");
  }

  calc.milliseconds = result.milliseconds;
  calc.result = result.area;
  calc.state = TaskState::Completed;
  process.buffer.clear();
  process.current_calc_name.clear();
  startNextCalculation(app, process);
}

void chernov::checkProcesses(App &app)
{
  int wait_error = 0;
  for (auto &entry : app.processes) {
    process_t &process = entry.second;
    if (process.state == ProcessState::Stopped) {
      continue;
    }

    if (process.result_fd >= 0) {
      try {
        receiveAvailableResult(app, process);
      } catch (...) {
      }
    }

    pid_t result;
    do {
      result = waitpid(process.pid, nullptr, WNOHANG);
    } while (result == -1 && errno == EINTR);
    if (result == 0) {
      continue;
    }
    if (result == -1 && errno != ECHILD) {
      wait_error = errno;
      continue;
    }

    for (auto &calc_entry : app.calcs) {
      calc_t &calc = calc_entry.second;
      if (calc.process_name == entry.first && (calc.state == TaskState::Pending || calc.state == TaskState::Running)) {
        calc.state = TaskState::Failed;
      }
    }
    if (process.task_fd >= 0) {
      close(process.task_fd);
    }
    if (process.result_fd >= 0) {
      close(process.result_fd);
    }
    process.task_fd = -1;
    process.result_fd = -1;
    process.buffer.clear();
    process.pending_calcs.clear();
    process.current_calc_name.clear();
    process.state = ProcessState::Stopped;
  }
  if (wait_error != 0) {
    throw std::runtime_error(std::strerror(wait_error));
  }
}

void chernov::shutdownProcesses(App &app)
{
  for (auto &entry : app.processes) {
    process_t &process = entry.second;
    if (process.state == ProcessState::Stopped) {
      continue;
    }

    process.state = ProcessState::Stopping;
    if (process.task_fd >= 0) {
      close(process.task_fd);
      process.task_fd = -1;
    }
    for (const std::string &calc_name : process.pending_calcs) {
      calc_t &calc = app.calcs.at(calc_name);
      if (calc.state == TaskState::Pending) {
        calc.state = TaskState::Failed;
      }
    }
    process.pending_calcs.clear();
  }

  int wait_error = 0;
  for (auto &entry : app.processes) {
    process_t &process = entry.second;
    if (process.state == ProcessState::Stopped) {
      continue;
    }

    pid_t result;
    do {
      result = waitpid(process.pid, nullptr, 0);
    } while (result == -1 && errno == EINTR);
    if (result == -1 && errno != ECHILD) {
      wait_error = errno;
    }

    if (process.result_fd >= 0) {
      try {
        receiveAvailableResult(app, process);
      } catch (...) {
      }
    }
    if (!process.current_calc_name.empty()) {
      calc_t &calc = app.calcs.at(process.current_calc_name);
      if (calc.state == TaskState::Running) {
        calc.state = TaskState::Failed;
      }
    }
    if (process.result_fd >= 0) {
      close(process.result_fd);
    }
    process.result_fd = -1;
    process.buffer.clear();
    process.current_calc_name.clear();
    process.state = ProcessState::Stopped;
  }
  if (wait_error != 0) {
    throw std::runtime_error(std::strerror(wait_error));
  }
}

bool chernov::isInside(const double x, const double y, const ellipse_t &ellipse)
{
  const double dx = x - ellipse.center.x;
  const double dy = y - ellipse.center.y;
  const double cosine = std::cos(ellipse.ksi);
  const double sine = std::sin(ellipse.ksi);
  const double local_x = dx * cosine + dy * sine;
  const double local_y = -dx * sine + dy * cosine;
  const double a = ellipse.horizontal_radius;
  const double b = ellipse.vertical_radius;
  return local_x * local_x / (a * a) + local_y * local_y / (b * b) <= 1;
}

bool chernov::isInsideAny(point_t point, const std::vector< ellipse_t > &ellipses)
{
  for (const ellipse_t &ellipse : ellipses) {
    if (isInside(point.x, point.y, ellipse)) {
      return true;
    }
  }
  return false;
}

size_t chernov::calc(
    const std::vector< ellipse_t > &ellipses, const frame_t &frame, const size_t tests, const size_t seed)
{
  std::default_random_engine gen(seed);
  constexpr double random_min = std::default_random_engine::min();
  constexpr double random_range = static_cast< double >(std::default_random_engine::max()) - random_min + 1;
  const double x_scale = (frame.right_top.x - frame.left_bottom.x) / random_range;
  const double y_scale = (frame.right_top.y - frame.left_bottom.y) / random_range;
  const std::vector< detail::prepared_ellipse_t > prepared = detail::prepareEllipses(ellipses);

  size_t count = 0;
  for (size_t i = 0; i < tests; ++i) {
    const double x = frame.left_bottom.x + (gen() - random_min) * x_scale;
    const double y = frame.left_bottom.y + (gen() - random_min) * y_scale;

    for (const detail::prepared_ellipse_t &ellipse : prepared) {
      const double dx = x - ellipse.x;
      const double dy = y - ellipse.y;
      if (ellipse.xx * dx * dx + ellipse.xy * dx * dy + ellipse.yy * dy * dy <= 1) {
        ++count;
        break;
      }
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

std::vector< chernov::detail::prepared_ellipse_t > chernov::detail::prepareEllipses(
    const std::vector< ellipse_t > &ellipses)
{
  std::vector< prepared_ellipse_t > prepared;
  prepared.reserve(ellipses.size());
  for (const ellipse_t &ellipse : ellipses) {
    const double cosine = std::cos(ellipse.ksi);
    const double sine = std::sin(ellipse.ksi);
    const double inverse_horizontal = 1.0 / (ellipse.horizontal_radius * ellipse.horizontal_radius);
    const double inverse_vertical = 1.0 / (ellipse.vertical_radius * ellipse.vertical_radius);
    prepared.push_back({ellipse.center, cosine * cosine * inverse_horizontal + sine * sine * inverse_vertical,
        2 * cosine * sine * (inverse_horizontal - inverse_vertical),
        sine * sine * inverse_horizontal + cosine * cosine * inverse_vertical});
  }
  return prepared;
}
