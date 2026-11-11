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

    size_t countHits(const std::vector< circle_t >& circles,
        const rectangle_t& frame, size_t tries, unsigned int seed);

    void* runThread(void* data);
  }
}

size_t studilova::detail::countHits(const std::vector< circle_t >& circles,
    const rectangle_t& frame, size_t tries, unsigned int seed)
{
  std::default_random_engine engine(seed);
  std::uniform_real_distribution< double > distributionX(frame.minX, frame.maxX);
  std::uniform_real_distribution< double > distributionY(frame.minY, frame.maxY);

  size_t hits = 0;

  for (size_t i = 0; i < tries; ++i)
  {
    const double x = distributionX(engine);
    const double y = distributionY(engine);

    if (isInside(x, y, circles))
    {
      ++hits;
    }
  }

  return hits;
}

void* studilova::detail::runThread(void* data)
{
  thread_data_t* threadData = static_cast< thread_data_t* >(data);

  threadData->hits = countHits(
      *threadData->circles,
      threadData->frame,
      threadData->tries,
      threadData->seed);

  return nullptr;
}

double studilova::calculateArea(const std::vector< circle_t >& circles,
    size_t threads, size_t tries, unsigned int seed)
{
  if (threads == 0 || tries == 0)
  {
    throw std::invalid_argument("Threads and tries must be positive");
  }

  const rectangle_t frame = findFrame(circles);
  const size_t partSize = tries / threads;
  const size_t remainder = tries % threads;

  std::vector< pthread_t > th(threads);
  std::vector< detail::thread_data_t > threadData(threads);

  std::size_t created = 0;
  int createError = 0;

  for (size_t i = 0; i < threads; ++i)
  {
    threadData[i].circles = &circles;
    threadData[i].frame = frame;
    threadData[i].tries = partSize;

    if (i == threads - 1)
    {
      threadData[i].tries += remainder;
    }

    threadData[i].seed = seed + static_cast< unsigned int >(i);
    threadData[i].hits = 0;

    const int error = pthread_create(&th[i], nullptr, detail::runThread, &threadData[i]);

    if (error != 0)
    {
      createError = error;
      break;
    }

    ++created;
  }

  size_t totalHits = 0;
  for (size_t i = 0; i < created; ++i)
  {
    const int error = pthread_join(th[i], nullptr);
    if (error != 0)
    {
      std::terminate();
    }
    totalHits += threadData[i].hits;
  }

  if (createError != 0)
  {
    throw std::runtime_error(std::strerror(createError));
  }

  const double width = frame.maxX - frame.minX;
  const double height = frame.maxY - frame.minY;
  const double rectangleArea = width * height;
  const double hitRatio = static_cast< double >(totalHits) / tries;

  return rectangleArea * hitRatio;
}
