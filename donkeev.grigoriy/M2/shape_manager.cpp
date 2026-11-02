#include "shape_manager.hpp"
#include <stdexcept>

void donkeev::ShapeManager::addCircle(const std::string& name, int r, int x, int y)
{
  if (circles_.count(name) > 0)
  {
    throw std::invalid_argument("Circle with this name already exists");
  }
  circles_.emplace(name, circle_t(r, x, y));
}

void donkeev::ShapeManager::addSet(const std::string& name, const std::vector< std::string >& shapeNames)
{
  if (sets_.count(name) > 0)
  {
    throw std::invalid_argument("Set with this name already exists");
  }

  for (const auto& shapeName : shapeNames)
  {
    if (circles_.count(shapeName) == 0)
    {
      throw std::invalid_argument("Referenced shape does not exist");
    }
  }

  sets_.emplace(name, shape_set_t{shapeNames});
}

const donkeev::circle_t& donkeev::ShapeManager::getCircle(const std::string& name) const
{
  auto it = circles_.find(name);
  if (it == circles_.end())
  {
    throw std::out_of_range("Circle does not exist");
  }
  return it->second;
}

const donkeev::shape_set_t& donkeev::ShapeManager::getSet(const std::string& name) const
{
  auto it = sets_.find(name);
  if (it == sets_.end())
  {
    throw std::out_of_range("Set does not exist");
  }
  return it->second;
}

bool donkeev::ShapeManager::hasCircle(const std::string& name) const
{
  return circles_.count(name) > 0;
}

bool donkeev::ShapeManager::hasSet(const std::string& name) const
{
  return sets_.count(name) > 0;
}

donkeev::rectangle_t donkeev::ShapeManager::getSetBoundingBox(const std::string& name) const
{
  auto it = sets_.find(name);
  if (it == sets_.end())
  {
    throw std::out_of_range("Set does not exist");
  }

  const auto& shapeNames = it->second.shape_names_;
  if (shapeNames.empty())
  {
    throw std::logic_error("Set is empty");
  }

  const auto& firstCircle = getCircle(shapeNames[0]);
  rectangle_t bbox = firstCircle.getBoundingBox();

  for (size_t i = 1; i < shapeNames.size(); ++i)
  {
    const auto& circle = getCircle(shapeNames[i]);
    rectangle_t currBbox = circle.getBoundingBox();

    if (currBbox.bottom_left_.x_ < bbox.bottom_left_.x_)
    {
      bbox.bottom_left_.x_ = currBbox.bottom_left_.x_;
    }
    if (currBbox.bottom_left_.y_ < bbox.bottom_left_.y_)
    {
      bbox.bottom_left_.y_ = currBbox.bottom_left_.y_;
    }
    if (currBbox.top_right_.x_ > bbox.top_right_.x_)
    {
      bbox.top_right_.x_ = currBbox.top_right_.x_;
    }
    if (currBbox.top_right_.y_ > bbox.top_right_.y_)
    {
      bbox.top_right_.y_ = currBbox.top_right_.y_;
    }
  }

  return bbox;
}
