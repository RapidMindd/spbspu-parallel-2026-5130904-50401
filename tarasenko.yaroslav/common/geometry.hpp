#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <stddef.h>

namespace tarasenko
{
  struct RealPoint
  {
    double x;
    double y;
  };

  struct Ellipse
  {
    double rx;
    double ry;
    RealPoint center;
  };

  struct Rectangle
  {
    RealPoint left_bottom;
    RealPoint right_top;
  };

  bool isInside(RealPoint point, Ellipse ellipse);
  Rectangle findFrame(const Ellipse* ellipses, size_t count);
  double getRectangleArea(Rectangle rectangle);
}

#endif
