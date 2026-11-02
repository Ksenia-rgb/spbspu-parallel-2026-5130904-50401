#include "area.hpp"

#include <functional>
#include <future>
#include <random>

namespace {
  bool isInsideAny(const std::vector< petrov::Shape >& shapes, double x, double y)
  {
    for (size_t i = 0; i < shapes.size(); ++i) {
      if (petrov::isInside(shapes[i], x, y)) {
        return true;
      }
    }
    return false;
  }

  size_t countHits(const std::vector< petrov::Shape >& shapes, const petrov::Frame& frame,
      size_t tries, size_t seed)
  {
    std::default_random_engine generator(seed);
    std::uniform_real_distribution< double > random_x(frame.left, frame.right);
    std::uniform_real_distribution< double > random_y(frame.bottom, frame.top);
    size_t hits = 0;
    for (size_t i = 0; i < tries; ++i) {
      const double x = random_x(generator);
      const double y = random_y(generator);
      if (isInsideAny(shapes, x, y)) {
        ++hits;
      }
    }
    return hits;
  }
}

double petrov::countArea(const std::vector< Shape >& shapes, size_t threads, size_t tries,
    size_t seed)
{
  const std::launch mode = std::launch::async;
  const Frame frame = getFrame(shapes);
  if (threads > tries) {
    threads = tries;
  }
  const size_t tries_per_thread = tries / threads;
  const size_t extra_tries = tries % threads;
  std::vector< std::future< size_t > > results;
  for (size_t i = 0; i < threads; ++i) {
    size_t part_tries = tries_per_thread;
    if (i < extra_tries) {
      ++part_tries;
    }
    results.push_back(std::async(mode, countHits, std::cref(shapes), frame, part_tries, seed + i));
  }
  size_t hits = 0;
  for (size_t i = 0; i < results.size(); ++i) {
    hits += results[i].get();
  }
  const double width = static_cast< double >(frame.right - frame.left);
  const double height = static_cast< double >(frame.top - frame.bottom);
  return width * height * static_cast< double >(hits) / static_cast< double >(tries);
}
