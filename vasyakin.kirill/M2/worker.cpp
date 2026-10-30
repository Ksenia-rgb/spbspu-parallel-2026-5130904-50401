#include <vector>
#include <unistd.h>
#include <cstddef>
#include <cstring>
#include <random>
#include <utility>
#include <pthread.h>
#include <algorithm>
#include "worker.hpp"
#include "structures.hpp"
#include "ipc.hpp"
#include "clicker.hpp"

namespace
{
  struct Thread
  {
    std::vector< vasyakin::circle_t > circles;
    size_t tries;
    unsigned int seed;
    size_t union_hits;
  };

  void* threadFunc(void* data)
  {
    auto* param = static_cast< Thread* >(data);
    param->union_hits = vasyakin::calc(param->tries, param->seed, param->circles);
    return nullptr;
  }
}

bool vasyakin::isInside(double x, double y, const vasyakin::circle_t& circle)
{
  const double dx = circle.x - x;
  const double dy = circle.y - y;
  const double r = static_cast< double >(circle.r);

  return dx * dx + dy * dy <= r * r;
}

vasyakin::rectangle_t vasyakin::findRectangle(const std::vector< vasyakin::circle_t >& circles)
{
  int min_x = circles[0].x - circles[0].r;
  int min_y = circles[0].y - circles[0].r;
  int max_x = circles[0].x + circles[0].r;
  int max_y = circles[0].y + circles[0].r;

  for (size_t i = 1; i < circles.size(); ++i)
  {
    min_x = std::min(circles[i].x - circles[i].r, min_x);
    min_y = std::min(circles[i].y - circles[i].r, min_y);
    max_x = std::max(circles[i].x + circles[i].r, max_x);
    max_y = std::max(circles[i].y + circles[i].r, max_y);
  }

  vasyakin::rectangle_t rect{min_x, max_x, min_y, max_y};
  return rect;
}

size_t vasyakin::calc(size_t tries, unsigned int seed, const std::vector< vasyakin::circle_t >& circles)
{
  size_t count = 0;

  const vasyakin::rectangle_t rect{vasyakin::findRectangle(circles)};

  std::default_random_engine engine(seed);
  std::uniform_real_distribution< double > dist1(rect.min_x, rect.max_x);
  std::uniform_real_distribution< double > dist2(rect.min_y, rect.max_y);

  for (size_t i = 0; i < tries; ++i)
  {
    const double x = dist1(engine);
    const double y = dist2(engine);

    for (size_t j = 0; j < circles.size(); ++j)
    {
      if (vasyakin::isInside(x, y, circles[j]))
      {
        ++count;
        break;
      }
    }
  }

  return count;
}

double vasyakin::area(size_t threads, size_t tries, unsigned int seed, const std::vector< vasyakin::circle_t >& circles,
    long long& time_ms)
{
  const size_t chunk = tries / threads;
  const size_t remainder = tries % threads;

  std::vector< pthread_t > pths(threads);
  std::vector< Thread > th(threads);

  size_t count_th = 0;

  size_t count_in_one_circle = 0;
  bool has_error = false;

  const vasyakin::Clicker timer;

  for (size_t i = 0; i < threads; ++i)
  {
    th[i].seed = seed + static_cast< unsigned int >(i);
    th[i].circles = circles;
    th[i].tries = (i == threads - 1) ? chunk + remainder : chunk;

    const int err = pthread_create(&pths[i], nullptr, threadFunc, &th[i]);
    if (err)
    {
      has_error = true;
      break;
    }

    ++count_th;
  }

  for (size_t i = 0; i < count_th; ++i)
  {
    const int err = pthread_join(pths[i], nullptr);

    if (err)
    {
      has_error = true;
      break;
    }
    else if (!has_error)
    {
      count_in_one_circle += th[i].union_hits;
    }
  }

  time_ms = static_cast< long long >(timer.millisec());

  if (has_error)
  {
    return 0.0;
  }

  const vasyakin::rectangle_t rect{vasyakin::findRectangle(circles)};
  const double rect_area
      = static_cast< double >(rect.max_x - rect.min_x) * static_cast< double >(rect.max_y - rect.min_y);

  const double area_union = static_cast< double >(count_in_one_circle) / tries * rect_area;

  return area_union;
}

int vasyakin::worker(int cmd_fd, int res_fd, unsigned int seed)
{
  while (true)
  {
    vasyakin::CommandHeader header;
    int err = 0;
    size_t rec = recv(err, cmd_fd, reinterpret_cast< char* >(&header), sizeof(header));

    if (rec != sizeof(header))
    {
      close(cmd_fd);
      close(res_fd);

      return (err < 0) ? 1 : 0;
    }

    if (header.type == CommandType::SHUTDOWN)
    {
      close(cmd_fd);
      close(res_fd);

      return 0;
    }

    std::vector< circle_t > circles;
    circles.reserve(header.shape_count);

    size_t total_rec = 0;

    while (total_rec < header.shape_count)
    {
      ShapeBatch batch;

      rec = recv(err, cmd_fd, reinterpret_cast< char* >(&batch), sizeof(batch));

      if (rec != sizeof(batch))
      {
        close(cmd_fd);
        close(res_fd);

        return (err < 0) ? 1 : 0;
      }

      for (size_t i = 0; i < batch.count; ++i)
      {
        circles.push_back(batch.shapes[i]);
      }

      total_rec += batch.count;
    }

    long long time_ms = 0;
    auto calc_result = vasyakin::area(header.threads, header.tries, seed, circles, time_ms);

    Result result;

    std::memset(&result, 0, sizeof(result));
    std::strncpy(result.calc_name, header.calc_name, MAX_NAME_LEN - 1);

    result.area = calc_result;
    result.time_ms = time_ms;

    const size_t sen = send(err, res_fd, reinterpret_cast< char* >(&result), sizeof(result));
    if (sen != sizeof(result))
    {
      close(cmd_fd);
      close(res_fd);

      return (err < 0) ? 1 : 0;
    }
  }
}
