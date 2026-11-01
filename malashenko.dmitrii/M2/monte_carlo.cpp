#include "monte_carlo.hpp"

#include <algorithm>
#include <future>
#include <random>
#include <stdexcept>
#include <vector>



size_t malashenko::calc(const CalculationParams &params)
{
  std::default_random_engine generator(params.seed);

  std::uniform_real_distribution<double> x_distribution(params.box.min_x, params.box.max_x);
  std::uniform_real_distribution<double> y_distribution(params.box.min_y, params.box.max_y);

  size_t inside_any = 0;

  for (size_t i = 0; i < params.tries; ++i)
  {
    const double x = x_distribution(generator);
    const double y = y_distribution(generator);

    bool inside = false;

    for (const Circle &circle : params.circles)
    {
      if (isInside(circle, x, y))
      {
        inside = true;
        break;
      }
    }

    inside_any += inside;
  }

  return inside_any;
}

double malashenko::getArea(const BoundingBox &box, size_t tries, size_t inside_any)
{
  const double width = static_cast<double>(box.max_x) - box.min_x;

  const double height = static_cast<double>(box.max_y) - box.min_y;

  const double box_area = width * height;

  return box_area * (static_cast<double>(inside_any) / tries);
}

size_t malashenko::calcParallel(const CalculationParams &params)
{
  const size_t actual_threads = std::min(params.threads, params.tries);

  const size_t tries_per_thread = params.tries / actual_threads;

  const size_t remainder = params.tries % actual_threads;

  std::vector<std::future<size_t>> futures;

  futures.reserve(actual_threads);

  for (size_t i = 0; i < actual_threads; ++i)
  {
    const size_t thread_tries = tries_per_thread + (i == actual_threads - 1 ? remainder : 0);

    CalculationParams thread_params = params;

    thread_params.tries = thread_tries;
    thread_params.seed += i;

    futures.emplace_back(std::async(std::launch::async, calc, thread_params));
  }

  size_t inside_any = 0;

  for (auto &future : futures)
  {
    inside_any += future.get();
  }

  return inside_any;
}

double malashenko::calculateArea(CalculationParams &params)
{
  if (params.circles.empty() || params.threads == 0 || params.tries == 0)
  {
    throw std::invalid_argument("Invalid calculation parameters");
  }

  const BoundingBox box = getBoundingBox(params.circles);

  params.box = box;

  const size_t inside_any = calcParallel(params);

  return getArea(box, params.tries, inside_any);
}