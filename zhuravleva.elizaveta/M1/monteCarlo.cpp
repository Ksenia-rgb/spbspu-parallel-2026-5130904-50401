#include "monteCarlo.hpp"
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <random>

bool zhuravleva::isInside(const Circle& circle, double x, double y)
{
  const double dx = x - circle.x;
  const double dy = y - circle.y;
  const double rad = circle.radius;
  return dx * dx + dy * dy <= rad * rad;
}

zhuravleva::Bounds zhuravleva::getBounds(const std::vector<Circle>& circles)
{
  const Circle& first = circles.front();
  Bounds bounds {
    static_cast< double >(first.x) - first.radius,
    static_cast< double >(first.x) + first.radius,
    static_cast< double >(first.y) - first.radius,
    static_cast< double >(first.y) + first.radius
  };
  for (std::size_t i = 1; i < circles.size(); i++)
  {
    const Circle& circle = circles[i];
    bounds.minX = std::min(bounds.minX, static_cast< double >(circle.x) - circle.radius);
    bounds.maxX = std::max(bounds.maxX, static_cast< double >(circle.x) + circle.radius);
    bounds.minY = std::min(bounds.minY, static_cast< double >(circle.y) - circle.radius);
    bounds.maxY = std::max(bounds.maxY, static_cast< double >(circle.y) + circle.radius);
  }
  return bounds;
}

std::vector< zhuravleva::Circle > zhuravleva::readCircles(std::istream& in)
{
  std::vector< Circle > circles;
  while (true)
  {
    int radius = 0, ignored = 0;
    int x = 0, y = 0;

    if (!(in >> radius))
    {
      if (in.eof())
      {
        break;
      }
      throw std::invalid_argument("Invalid circle input");
    }
    if (!(in >> ignored >> x >> y))
    {
      throw std::invalid_argument("Incomplete circle input");
    }
    circles.push_back(Circle{radius, x, y});
  }
  return circles;
}

zhuravleva::Hits zhuravleva::checkPoint(const std::vector< Circle >& circles, double x, double y)
{
  bool inAny = false;
  bool inAll = true;

  for (const Circle& circle : circles)
  {
    const bool inside = isInside(circle, x, y);
    inAny = inAny || inside;
    inAll = inAll && inside;
  }

  return Hits{inAny, inAll};
}

zhuravleva::Hits zhuravleva::calcMonte(
    const std::vector< Circle >& circles, const Bounds& bounds, std::size_t tries, std::size_t seed)
{
  std::default_random_engine generator(seed);
  std::uniform_real_distribution< double > distX(bounds.minX, bounds.maxX);
  std::uniform_real_distribution< double > distY(bounds.minY, bounds.maxY);

  Hits total{0, 0};

  for (std::size_t i = 0; i < tries; ++i)
  {
    const double x = distX(generator);
    const double y = distY(generator);

    const Hits result = checkPoint(circles, x, y);
    total.any += result.any;
    total.all += result.all;
  }

  return total;
}
