#include "area.hpp"
#include <future>
#include <random>
#include <thread>
#include <utility>
#include <vector>

namespace tarasenko
{
  namespace
  {
    std::pair< long, long > calc(const Ellipse* ellipses, size_t count, Rectangle frame, long tries, long long seed)
    {
      std::default_random_engine gen(seed);
      const double range = gen.max() - gen.min() + 1.0;
      const double kx = (frame.right_top.x - frame.left_bottom.x) / (range * range);
      const double ky = (frame.right_top.y - frame.left_bottom.y) / (range * range);
      long union_hits = 0;
      long intersection_hits = 0;
      for (long long i = 0; i < tries; ++i)
      {
        bool is_inside_all = true;
        bool is_first_hit = true;
        double x = gen() - gen.min();
        x += (gen() - gen.min()) * range;
        double y = gen() - gen.min();
        y += (gen() - gen.min()) * range;
        const RealPoint point = {frame.left_bottom.x + x * kx, frame.left_bottom.y + y * ky};
        for (size_t j = 0; j < count; ++j)
        {
          if (isInside(point, ellipses[j]))
          {
            if (is_first_hit)
            {
              union_hits++;
              is_first_hit = false;
            }
            if (!is_inside_all)
            {
              break;
            }
          }
          else
          {
            is_inside_all = false;
          }
        }
        if (is_inside_all)
        {
          intersection_hits++;
        }
      }
      return {union_hits, intersection_hits};
    }

    void promiseValueSetter(const Ellipse* ellipses, size_t count, Rectangle frame, long tries, long long seed,
        std::promise< std::pair< long, long > > p)
    {
      p.set_value(calc(ellipses, count, frame, tries, seed));
    }
  }

  std::pair< double, double > getArea(const Ellipse* ellipses, size_t count, long threads, long tries, long long seed)
  {
    long tries_per_thread = tries / threads;
    Rectangle frame = findFrame(ellipses, count);
    std::vector< std::thread > descriptors;
    std::vector< std::future< std::pair< long, long > > > futures;
    for (long long i = 0; i < threads - 1; ++i)
    {
      std::promise< std::pair< long, long > > p;
      futures.push_back(p.get_future());
      descriptors.emplace_back(promiseValueSetter, ellipses, count, frame, tries_per_thread, seed + i, std::move(p));
    }
    std::promise< std::pair< long, long > > p;
    futures.push_back(p.get_future());
    descriptors.emplace_back(promiseValueSetter, ellipses, count, frame, tries_per_thread + (tries % threads),
        seed + threads - 1, std::move(p));

    long long total_union = 0;
    long long total_intersection = 0;
    for (size_t i = 0; i < futures.size(); ++i)
    {
      auto res = futures[i].get();
      total_union += res.first;
      total_intersection += res.second;
    }
    for (size_t i = 0; i < descriptors.size(); ++i)
    {
      descriptors[i].join();
    }

    const double area = getRectangleArea(frame);
    return {area * total_union / tries, area * total_intersection / tries};
  }
}
