#include "geom_db.hpp"

namespace levkin {
  void ShapeSet::updateBoxBoundary()
  {}

  bool ShapeSet::checkHit(p_t)
  {
    return false;
  }

  void Circle::updateBoxBoundary()
  {
    boxBoundary.bottomLeft = {center.x - radius, center.y - radius};
    boxBoundary.topRight = {center.x + radius, center.y + radius};
  }

  bool Circle::checkHit(p_t c)
  {

    double dx = c.x - center.x;
    double dy = c.y - center.y;

    return dx * dx + dy * dy <= radius * static_cast< double >(radius);
  }

  void Elipse::updateBoxBoundary()
  {}

  bool Elipse::checkHit(p_t)
  {
    return false;
  }

}
