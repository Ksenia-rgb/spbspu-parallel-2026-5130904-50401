#include "circle.hpp"
#include <stdexcept>
#include <vector>

madieva::Circle::Circle(long long r, long long cx, long long cy)
{
  if (r <= 0)
  {
    throw std::invalid_argument("a radius must be positive");
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

madieva::frame_t madieva::computeFrame(const madieva::Circle & circle)
{
  const long long min_x = circle.getX() - circle.getRadius();
  const long long min_y = circle.getY() - circle.getRadius();
  const long long max_x = circle.getX() + circle.getRadius();
  const long long max_y = circle.getY() + circle.getRadius();
  return madieva::frame_t {min_x, min_y, max_x, max_y};
}

madieva::frame_t madieva::computeFrame(const std::vector< const madieva::Circle * > & figures)
{
  madieva::frame_t frame;
  frame.min_x = figures[0]->getX() - figures[0]->getRadius();
  frame.max_x = figures[0]->getX() + figures[0]->getRadius();
  frame.min_y = figures[0]->getY() - figures[0]->getRadius();
  frame.max_y = figures[0]->getY() + figures[0]->getRadius();
  long long min_x = 0;
  long long max_x = 0;
  long long min_y = 0;
  long long max_y = 0;
  for (std::size_t i = 1; i < figures.size(); ++i)
  {
    min_x = figures[i]->getX() - figures[i]->getRadius();
    max_x = figures[i]->getX() + figures[i]->getRadius();
    min_y = figures[i]->getY() - figures[i]->getRadius();
    max_y = figures[i]->getY() + figures[i]->getRadius();
    if (frame.min_x > min_x)
    {
      frame.min_x = min_x;
    }
    if (frame.max_x < max_x)
    {
      frame.max_x = max_x;
    }
    if (frame.min_y > min_y)
    {
      frame.min_y = min_y;
    }
    if (frame.max_y < max_y)
    {
      frame.max_y = max_y;
    }
  }
  return frame;
}

std::ostream & madieva::operator<<(std::ostream & out, const madieva::frame_t & frame)
{
  out << "(" << frame.min_x << " " << frame.min_y << ") (" << frame.max_x << " " << frame.max_y << ")";
  return out;
}

