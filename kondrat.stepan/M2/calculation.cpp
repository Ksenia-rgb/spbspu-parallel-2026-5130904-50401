#include "calculation.hpp"
#include <algorithm>
#include <cmath>
#include <functional>
#include <future>
#include <stdexcept>

namespace
{
  bool contains(const kondrat::Shape & shape, double x, double y)
  {
    const double dx = x - shape.x;
    const double dy = y - shape.y;
    const double cosine = std::cos(shape.angle);
    const double sine = std::sin(shape.angle);
    const double local_x = (dx * cosine + dy * sine) / shape.horizontal_radius;
    const double local_y = (-dx * sine + dy * cosine) / shape.vertical_radius;

    return local_x * local_x + local_y * local_y <= 1.0;
  }

  long long countHits(
    const std::vector< kondrat::Shape > & shapes,
    const kondrat::Rectangle & rectangle,
    long long tries,
    std::default_random_engine::result_type seed)
  {
    std::default_random_engine generator(seed);
    std::uniform_real_distribution< double > x_distribution(rectangle.left, rectangle.right);
    std::uniform_real_distribution< double > y_distribution(rectangle.bottom, rectangle.top);
    long long hits = 0;

    for (long long i = 0; i < tries; ++i)
    {
      const double x = x_distribution(generator);
      const double y = y_distribution(generator);

      for (const kondrat::Shape & shape : shapes)
      {
        if (contains(shape, x, y))
        {
          ++hits;
          break;
        }
      }
    }

    return hits;
  }
}

double kondrat::calculateArea(
  const std::vector< Shape > & shapes,
  long long threads,
  long long tries,
  std::default_random_engine & generator)
{
  if (shapes.empty() || threads < 0 || tries <= 0)
  {
    throw std::invalid_argument("Invalid calculation parameters");
  }

  Rectangle rectangle = findFrame(shapes.front());

  for (const Shape & shape : shapes)
  {
    const Rectangle frame = findFrame(shape);

    rectangle.left = std::min(rectangle.left, frame.left);
    rectangle.right = std::max(rectangle.right, frame.right);
    rectangle.bottom = std::min(rectangle.bottom, frame.bottom);
    rectangle.top = std::max(rectangle.top, frame.top);
  }

  const long long task_count = threads == 0 ? 1 : threads;
  const long long base_tries = tries / task_count;
  const long long remainder = tries % task_count;
  std::vector< std::future< long long > > futures;

  for (long long i = 0; i < task_count; ++i)
  {
    const long long task_tries = base_tries + (i < remainder ? 1 : 0);
    const std::default_random_engine::result_type task_seed = generator();

    futures.push_back(std::async(
      std::launch::async, countHits, std::cref(shapes), std::cref(rectangle), task_tries, task_seed));
  }

  long long hits = 0;

  for (std::future< long long > & future : futures)
  {
    hits += future.get();
  }

  const double rectangle_area = (rectangle.right - rectangle.left) * (rectangle.top - rectangle.bottom);

  return rectangle_area * (static_cast< double >(hits) / tries);
}
