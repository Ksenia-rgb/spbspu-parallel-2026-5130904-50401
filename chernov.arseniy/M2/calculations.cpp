#include "calculations.hpp"

#include "geometry.hpp"

#include <algorithm>
#include <functional>
#include <future>
#include <random>
#include <unistd.h>

bool chernov::read_all(int fd, void *buffer, size_t size)
{
  auto *ptr = static_cast< char * >(buffer);

  while (size > 0) {
    const ssize_t n = read(fd, ptr, size);

    if (n == -1 && errno == EINTR) {
      continue;
    }

    if (n <= 0) {
      return false;
    }

    ptr += n;
    size -= n;
  }

  return true;
}

bool chernov::write_all(int fd, const void *buffer, size_t size)
{
  auto *ptr = static_cast< const char * >(buffer);
  while (size > 0) {
    const ssize_t n = write(fd, ptr, size);

    if (n == -1 && errno == EINTR) {
      continue;
    }

    if (n == 0) {
      return false;
    }

    ptr += n;
    size -= n;
  }

  return true;
}

void chernov::calculationLoop(int task_fd, int result_fd, size_t seed, const void *shared_memory, size_t shared_bytes)
{
  while (true) {
    Task task{};

    if (!read_all(task_fd, &task, sizeof(task))) {
      break;
    }

    const auto *bytes = static_cast< const unsigned char * >(shared_memory);
    const auto *values = reinterpret_cast< const ellipse_t * >(bytes + task.offset);

    std::vector< ellipse_t > ellipses;

    if (task.count != 0) {
      ellipses.assign(values, values + task.count);
    }

    const double area = areaOf(ellipses, task.threads, task.tries, seed);

    if (!write_all(result_fd, &area, sizeof(area))) {
      close(task_fd);
      close(result_fd);
      _exit(1);
    }
  }
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
