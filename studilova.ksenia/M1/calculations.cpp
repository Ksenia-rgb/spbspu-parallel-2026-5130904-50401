#include "calculations.hpp"

#include <algorithm>
#include <random>
#include <functional>
#include <future>
#include <stdexcept>
#include <utility>
#include <limits>
#include <string>
#include <cstddef>
#include <vector>

bool studilova::containsPoint(const Circle& circle, double x, double y)
{
  const double dx = x - circle.x;
  const double dy = y - circle.y;

  return (dx * dx + dy * dy) <= (circle.radius * circle.radius);
}

studilova::Rectangle studilova::findRectangle(const std::vector< Circle >& circles)
{
  if (circles.empty())
  {
    return Rectangle{0, 0, 0, 0};
  }

  const Circle& first = circles.front();
  const double min_x = first.x - first.radius;
  const double min_y = first.y - first.radius;
  const double max_x = first.x + first.radius;
  const double max_y = first.y + first.radius;
  Rectangle rectangle = {min_x, min_y, max_x, max_y};

  for (const Circle& circle : circles)
  {
    rectangle.min_x = std::min(rectangle.min_x, circle.x - circle.radius);
    rectangle.min_y = std::min(rectangle.min_y, circle.y - circle.radius);
    rectangle.max_x = std::max(rectangle.max_x, circle.x + circle.radius);
    rectangle.max_y = std::max(rectangle.max_y, circle.y + circle.radius);
  }

  return rectangle;
}

std::pair< size_t, size_t > studilova::countHits(
    const std::vector< Circle >& circles, const Rectangle& rectangle, size_t tries, size_t seed)
{
  if (circles.empty())
  {
    return {0, 0};
  }

  std::default_random_engine engine(seed);
  std::uniform_real_distribution< double > distribution_x(rectangle.min_x, rectangle.max_x);
  std::uniform_real_distribution< double > distribution_y(rectangle.min_y, rectangle.max_y);

  size_t union_hits = 0;
  size_t intersection_hits = 0;

  for (size_t i = 0; i < tries; ++i)
  {
    const double x = distribution_x(engine);
    const double y = distribution_y(engine);

    bool in_any = false;
    bool in_all = true;

    for (const Circle& circle : circles)
    {
      if (containsPoint(circle, x, y))
      {
        in_any = true;
      }
      else
      {
        in_all = false;
      }
    }

    if (in_any)
    {
      ++union_hits;
    }

    if (in_all)
    {
      ++intersection_hits;
    }
  }

  return {union_hits, intersection_hits};
}

std::pair< double, double > studilova::calculateAreas(
    const std::vector< Circle >& circles, size_t thread_count, size_t tries, size_t seed)
{
  if (tries == 0)
  {
    throw std::invalid_argument("Number of tries must be positive");
  }

  if (circles.empty())
  {
    return {0, 0};
  }

  if (thread_count == 0)
  {
    thread_count = 1;
  }

  const Rectangle rectangle = findRectangle(circles);
  const size_t part_size = tries / thread_count;
  const size_t remainder = tries % thread_count;

  using Hits = std::pair< std::size_t, std::size_t >;
  std::vector< std::future< Hits > > futures;
  futures.reserve(thread_count);

  for (size_t i = 0; i < thread_count; ++i)
  {
    const size_t extra = (i < remainder) ? 1 : 0;
    const size_t local_tries = part_size + extra;
    const size_t local_seed = seed + i;

    auto future
        = std::async(std::launch::async, countHits, std::cref(circles), std::cref(rectangle), local_tries, local_seed);
    futures.push_back(std::move(future));
  }

  size_t union_hits = 0;
  size_t intersection_hits = 0;

  for (std::future< Hits >& future : futures)
  {
    const Hits hits = future.get();
    union_hits += hits.first;
    intersection_hits += hits.second;
  }

  const double width = rectangle.max_x - rectangle.min_x;
  const double height = rectangle.max_y - rectangle.min_y;
  const double rectangle_area = width * height;

  const double union_part = static_cast< double >(union_hits) / static_cast< double >(tries);
  const double intersection_part = static_cast< double >(intersection_hits) / static_cast< double >(tries);

  return {rectangle_area * union_part, rectangle_area * intersection_part};
}

size_t studilova::parseArgument(const char* text)
{
  const std::string argument = text;
  size_t pos = 0;
  const long long value = std::stoll(argument, &pos);

  if (pos != argument.size())
  {
    throw std::invalid_argument("Invalid characters in argument");
  }

  if (value < 0)
  {
    throw std::invalid_argument("Argument can not be negative");
  }

  if (static_cast< unsigned long long >(value) > std::numeric_limits< size_t >::max())
  {
    throw std::out_of_range("Argument is too large");
  }

  return static_cast< size_t >(value);
}
