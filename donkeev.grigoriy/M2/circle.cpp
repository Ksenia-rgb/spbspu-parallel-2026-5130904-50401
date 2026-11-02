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

donkeev::rectangle_t donkeev::circle_t::getBoundingBox() const
{
  point_t bottom_left{center_.x_ - radius_, center_.y_ - radius_};
  point_t top_right{center_.x_ + radius_, center_.y_ + radius_};
  
  return {bottom_left, top_right};
}
