#include "geometry.hpp"

namespace tarasenko
{
  bool isInside(RealPoint point, Ellipse ellipse)
  {
    point.x -= ellipse.center.x;
    point.y -= ellipse.center.y;
    return point.x * point.x / (ellipse.rx * ellipse.rx) + point.y * point.y / (ellipse.ry * ellipse.ry) <= 1;
  }

  Rectangle findFrame(const Ellipse* ellipses, size_t count)
  {
    const Ellipse& first = ellipses[0];
    double right_bound = first.center.x + first.rx;
    double left_bound = first.center.x - first.rx;
    double top_bound = first.center.y + first.ry;
    double bottom_bound = first.center.y - first.ry;
    for (size_t i = 0; i < count; ++i)
    {
      const Ellipse& cur = ellipses[i];
      if (right_bound < cur.center.x + cur.rx)
      {
        right_bound = cur.center.x + cur.rx;
      }
      if (left_bound > cur.center.x - cur.rx)
      {
        left_bound = cur.center.x - cur.rx;
      }
      if (top_bound < cur.center.y + cur.ry)
      {
        top_bound = cur.center.y + cur.ry;
      }
      if (bottom_bound > cur.center.y - cur.ry)
      {
        bottom_bound = cur.center.y - cur.ry;
      }
    }
    return Rectangle{{left_bound, bottom_bound}, {right_bound, top_bound}};
  }

  double getRectangleArea(Rectangle rectangle)
  {
    return (rectangle.right_top.x - rectangle.left_bottom.x) * (rectangle.right_top.y - rectangle.left_bottom.y);
  }
}
