#include "area.hpp"
#include <random>
#include <stdexcept>
#include <exception>
#include <cstring>
#include <pthread.h>

namespace studilova
{
  namespace detail
  {
    struct thread_data_t
    {
      const std::vector< circle_t >* circles;
      rectangle_t frame;
      size_t tries;
      unsigned int seed;
      size_t hits;
    };

    size_t countHits(const std::vector< circle_t >& circles, const rectangle_t& frame, size_t tries, unsigned int seed);

    void* runThread(void* data);
  }
}

size_t studilova::detail::countHits(
    const std::vector< circle_t >& circles, const rectangle_t& frame, size_t tries, unsigned int seed)
{
  std::default_random_engine engine(seed);
  const double range = static_cast< double >(engine.max() - engine.min()) + 1.0;
  const double width = frame.max_x - frame.min_x;
  const double height = frame.max_y - frame.min_y;

  size_t hits = 0;

  for (size_t i = 0; i < tries; ++i)
  {
    const double x = frame.min_x + width * static_cast< double >(engine() - engine.min()) / range;
    const double y = frame.min_y + height * static_cast< double >(engine() - engine.min()) / range;

    if (isInside(x, y, circles))
    {
      ++hits;
    }
  }
  return hits;
}

void* studilova::detail::runThread(void* data)
{
  thread_data_t* const thread_data = static_cast< thread_data_t* >(data);

  thread_data->hits = countHits(*thread_data->circles, thread_data->frame, thread_data->tries, thread_data->seed);

  return nullptr;
}

double studilova::calculateArea(const std::vector< circle_t >& circles, size_t threads, size_t tries, unsigned int seed)
{
  if (threads == 0 || tries == 0)
  {
    throw std::invalid_argument("Threads and tries must be positive");
  }

  const rectangle_t frame = findFrame(circles);
  const size_t partSize = tries / threads;
  const size_t remainder = tries % threads;

  std::vector< pthread_t > th(threads);
  std::vector< detail::thread_data_t > thread_data(threads);

  std::size_t created = 0;
  int create_error = 0;

  for (size_t i = 0; i < threads; ++i)
  {
    thread_data[i].circles = &circles;
    thread_data[i].frame = frame;
    thread_data[i].tries = partSize;

    if (i == threads - 1)
    {
      thread_data[i].tries += remainder;
    }

    thread_data[i].seed = seed + static_cast< unsigned int >(i);
    thread_data[i].hits = 0;

    const int error = pthread_create(&th[i], nullptr, detail::runThread, &thread_data[i]);

    if (error != 0)
    {
      create_error = error;
      break;
    }

    ++created;
  }

  size_t total_hits = 0;
  for (size_t i = 0; i < created; ++i)
  {
    const int error = pthread_join(th[i], nullptr);
    if (error != 0)
    {
      std::terminate();
    }
    total_hits += thread_data[i].hits;
  }

  if (create_error != 0)
  {
    throw std::runtime_error(std::strerror(create_error));
  }

  const double width = frame.max_x - frame.min_x;
  const double height = frame.max_y - frame.min_y;
  const double rectangleArea = width * height;
  const double hitRatio = static_cast< double >(total_hits) / tries;

  return rectangleArea * hitRatio;
}
