#ifndef SHAPE_VARIANT_HPP
#define SHAPE_VARIANT_HPP


#include "geom_db.hpp"


namespace levkin {
    enum ShapeType { CIRCLE, ELLIPSE };
  
    struct ShapeVariant {
      ShapeType shapeType;
      union {
        Circle circle;
        Elipse elipse;
      };
    };
}

#endif