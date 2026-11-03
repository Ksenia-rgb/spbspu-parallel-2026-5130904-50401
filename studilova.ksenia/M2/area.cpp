#include "area.hpp"
#include <random>
#include <stdexcept>

namespace studilova
{
  namespace detail
  {
    size_t countHits(const std::vector< circle_t >& circles,
        const rectangle_t& frame, size_t tries, unsigned int seed);
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

double studilova::calculateArea(const std::vector< circle_t >& circles,
    size_t tries, unsigned int seed)
{
  if (tries == 0)
  {
    throw std::invalid_argument("Number of tries must be positive");
  }

  const rectangle_t frame = findFrame(circles);
  const size_t hits = detail::countHits(circles, frame, tries, seed);

  const double width = frame.maxX - frame.minX;
  const double height = frame.maxY - frame.minY;
  const double rectangleArea = width * height;
  const double hitRatio = static_cast< double >(hits) / tries;

  return rectangleArea * hitRatio;
}