#include "circle.hpp"
#include <stdexcept>

madieva::Circle::Circle(long long r, long long cx, long long cy)
{
  if (r <= 0)
  {
    throw std::invalid_argument("a radius cannot be negative");
  }
  r_ = r;
  x_ = cx;
  y_ = cy;
}

long long madieva::Circle::getRadius() const
{
  return r_;
}

long long madieva::Circle::getX() const
{
  return x_;
}

long long madieva::Circle::getY() const
{
  return y_;
}

std::ostream & madieva::operator<<(std::ostream & out, const madieva::Circle & circle)
{
  out << circle.getRadius() << " (" << circle.getX() << " " << circle.getY() << ")";
  return out;
}
