#include "circle.hpp"
#include <stdexcept>

donkeev::circle_t::circle_t(int r, int x, int y):
  radius_(r),
  center_({x, y})
{
  if (r <= 0)
  {
    throw std::out_of_range("Radius must be positive");
  }
}

donkeev::point_t donkeev::circle_t::getCenter() const
{
  return center_;
}

int donkeev::circle_t::getRadius() const
{
  return radius_;
}