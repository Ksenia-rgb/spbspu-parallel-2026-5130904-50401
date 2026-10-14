#include "calculations.hpp"

#include <algorithm>
#include <random>
#include <functional>
#include <future>
#include <stdexcept>
#include <utility>
#include <limits>
#include <string>

bool studilova::containsPoint(const Circle& circle, double x, double y)
{
  double dx = x - circle.x;
  double dy = y - circle.y;

  return (dx * dx + dy * dy) <= (circle.radius * circle.radius);
}

studilova::Rectangle studilova::findRectangle(const std::vector< Circle >& circles)
{
  if (circles.empty())
  {
    return Rectangle{0, 0, 0, 0};
  }

  const Circle& first = circles.front();
  double minX = first.x - first.radius;
  double minY = first.y - first.radius;
  double maxX = first.x + first.radius;
  double maxY = first.y + first.radius;
  Rectangle rectangle = {minX, minY, maxX, maxY};

  for (const Circle& circle : circles)
  {
    rectangle.minX = std::min(rectangle.minX, circle.x - circle.radius);
    rectangle.minY = std::min(rectangle.minY, circle.y - circle.radius);
    rectangle.maxX = std::max(rectangle.maxX, circle.x + circle.radius);
    rectangle.maxY = std::max(rectangle.maxY, circle.y + circle.radius);
  }

  return rectangle;
}

std::pair< size_t, size_t > studilova::countHits(const std::vector< Circle >& circles,
    const Rectangle& rectangle, size_t tries, size_t seed)
{
  if (circles.empty())
  {
    return {0, 0};
  }

  std::default_random_engine engine(seed);
  std::uniform_real_distribution< double > distributionX(rectangle.minX, rectangle.maxX);
  std::uniform_real_distribution< double > distributionY(rectangle.minY, rectangle.maxY);

  size_t unionHits = 0;
  size_t intersectionHits = 0;

  for (size_t i = 0; i < tries; ++i)
  {
    double x = distributionX(engine);
    double y = distributionY(engine);

    bool inAny = false;
    bool inAll = true;

    for (const Circle& circle : circles)
    {
      if (containsPoint(circle, x, y))
      {
        inAny = true;
      }
      else
      {
        inAll = false;
      }
    }

    if (inAny)
    {
      ++unionHits;
    }

    if (inAll)
    {
      ++intersectionHits;
    }
  }

  return {unionHits, intersectionHits};
}

std::pair< double, double > studilova::calculateAreas(const std::vector< Circle >& circles,
    size_t threadCount, size_t tries, size_t seed)
{
  if (tries == 0)
  {
    throw std::invalid_argument("Number of tries must be positive");
  }

  if (circles.empty())
  {
    return {0, 0};
  }

  if (threadCount == 0)
  {
    threadCount = 1;
  }

  Rectangle rectangle = findRectangle(circles);
  size_t partSize = tries / threadCount;
  size_t remainder = tries % threadCount;

  using Hits = std::pair< std::size_t, std::size_t >;
  std::vector< std::future< Hits > > futures;
  futures.reserve(threadCount);

  for (size_t i = 0; i < threadCount; ++i)
  {
    size_t extra = (i < remainder) ? 1 : 0;
    size_t localTries = partSize + extra;
    size_t localSeed = seed + i;

    auto future = std::async(std::launch::async, countHits,
        std::cref(circles), std::cref(rectangle), localTries, localSeed);
    futures.push_back(std::move(future));
  }

  size_t unionHits = 0;
  size_t intersectionHits = 0;

  for (std::future< Hits >& future : futures)
  {
    Hits hits = future.get();
    unionHits += hits.first;
    intersectionHits += hits.second;
  }

  double width = rectangle.maxX - rectangle.minX;
  double height = rectangle.maxY - rectangle.minY;
  double rectangleArea = width * height;

  double unionPart = static_cast< double >(unionHits) / static_cast< double >(tries);
  double intersectionPart = static_cast< double >(intersectionHits) / static_cast< double >(tries);

  return {rectangleArea * unionPart, rectangleArea * intersectionPart};
}

size_t studilova::parseArgument(const char* text)
{
  const std::string argument = text;
  size_t pos = 0;
  long long value = std::stoll(argument, &pos);

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
