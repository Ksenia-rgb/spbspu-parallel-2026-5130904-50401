#ifndef GEOM_DB_HPP
#define GEOM_DB_HPP

#include "utils.hpp"
#include <unordered_map>
#include <memory>

namespace levkin {
  struct Shape {
    Rect boxBoundary;
    virtual void updateBoxBoundary() = 0;
    virtual bool checkHit(p_t p) const = 0;
    virtual ~Shape() = default;
  };

  struct Circle: Shape {
    p_t center;
    double radius;
    void updateBoxBoundary() override;
    bool checkHit(p_t p) const override;
  };

  struct Ellipse: Shape {
    p_t center;
    double radiusX;
    double radiusY;
    double angle = 0.0;
    void updateBoxBoundary() override;
    bool checkHit(p_t p) const override;
  };

  struct ShapeSet {
    std::string name;
    std::vector< std::string > shapeNames;
  };

  struct GeomDB {
    std::unordered_map< std::string, std::unique_ptr< Shape > > primitives;
    std::unordered_map< std::string, ShapeSet > sets;

    Rect getSetBoundingBox(const std::string& setName) const;
    bool checkHitSet(const std::string& setName, p_t p) const;
  };

}
#endif
