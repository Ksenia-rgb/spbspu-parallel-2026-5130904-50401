#ifndef SHAPE_MANAGER_HPP
#define SHAPE_MANAGER_HPP

#include <map>
#include <string>
#include <vector>
#include "circle.hpp"
#include "rectangle.hpp"
#include "shape_set.hpp"

namespace donkeev
{
  class ShapeManager
  {
  public:
    void addCircle(const std::string& name, int r, int x, int y);
    void addSet(const std::string& name, const std::vector<std::string>& shapeNames);

    const circle_t& getCircle(const std::string& name) const;
    const shape_set_t& getSet(const std::string& name) const;

    bool hasCircle(const std::string& name) const;
    bool hasSet(const std::string& name) const;

    rectangle_t getSetBoundingBox(const std::string& name) const;

  private:
    std::map<std::string, circle_t> circles_;
    std::map<std::string, shape_set_t> sets_;
  };
}

#endif
