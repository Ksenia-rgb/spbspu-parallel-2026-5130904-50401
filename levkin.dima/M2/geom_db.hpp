#ifndef GEOM_DB_HPP
#define GEOM_DB_HPP

#include "utils.hpp"
#include <unordered_map>

namespace levkin {
  struct Shape {
    Rect boxBoundary;
    virtual void updateBoxBoundary();
    virtual bool checkHit(p_t);
    virtual ~Shape() = default;
  };
  struct ShapeSet: Shape {
    std::vector< std::string > shapeNames;
    void updateBoxBoundary() override;
    bool checkHit(p_t) override;
    ~ShapeSet() override = default;
  };

  struct Circle: Shape {
    p_t center;
    double radius;
    void updateBoxBoundary() override;
    bool checkHit(p_t) override;
    ~Circle() override = default;
  };

  struct Elipse: Shape {
    p_t center;
    double radiusX;
    double radiusY;
    void updateBoxBoundary() override;
    bool checkHit(p_t) override;
    ~Elipse() override = default;
  };
  
  struct GeomDB {
    std::unordered_map< std::string, Circle > primitives;
    std::vector< ShapeSet > sets;
  };

}

#endif
