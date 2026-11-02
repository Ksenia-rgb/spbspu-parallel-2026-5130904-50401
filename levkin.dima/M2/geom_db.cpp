#include "geom_db.hpp"

namespace levkin {
  void Circle::updateBoxBoundary()
  {
    boxBoundary.bottomLeft = {
        center.x - radius,
        center.y - radius,
    };
    boxBoundary.topRight = {
        center.x + radius,
        center.y + radius,
    };
  }

  bool Circle::checkHit(p_t p) const
  {
    double dx = p.x - center.x;
    double dy = p.y - center.y;

    return dx * dx + dy * dy <= radius * static_cast< double >(radius);
  }

  void Ellipse::updateBoxBoundary()
  {
    return;
  }

  bool Ellipse::checkHit(p_t /*p*/) const
  {
    return false;
  }

  Rect GeomDB::getSetBoundingBox(const std::string& setName) const
  {
    auto it = sets.find(setName);
    if (it == sets.end()) {
      throw std::invalid_argument("didn't find such set");
    }
    for (const std::string& primitiveName : (*it).second.shapeNames) {
        auto it = primitives.find(primitiveName);
        if (it == primitives.name)
    }
    return Rect{};
  }

  bool GeomDB::checkHitSet(const std::string& /*setName*/, p_t p) const
  {
    // TODO: найти набор в sets, проверить checkHit(p) хотя бы для одной фигуры набора
    return false;
  }

}
