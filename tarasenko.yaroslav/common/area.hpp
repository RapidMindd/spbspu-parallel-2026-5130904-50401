#ifndef AREA_HPP
#define AREA_HPP

#include <utility>
#include "geometry.hpp"

namespace tarasenko
{
  std::pair< double, double > getArea(const Ellipse* ellipses, size_t count, long threads, long tries, long long seed);
}

#endif
