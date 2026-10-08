#include <cstddef>
#include <cstdlib>
#include <future>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <thread>
#include <utility>
#include <vector>
#include <algorithm>

namespace tarasenko {
  struct RealPoint {
    double x;
    double y;
  };

  struct Ellipse {
    double rx;
    double ry;
    RealPoint center;
  };

  struct Rectangle {
    RealPoint left_bottom;
    RealPoint right_top;
  };

  bool isInside(RealPoint point, Ellipse ellipse)
  {
    point.x -= ellipse.center.x;
    point.y -= ellipse.center.y;
    return point.x * point.x / (ellipse.rx * ellipse.rx) + point.y * point.y / (ellipse.ry * ellipse.ry) <= 1;
  }

  std::pair< long, long > calc(const std::vector< Ellipse >& ellipses, Rectangle frame, long tries, long long seed)
  {
    std::mt19937 gen(seed);
    std::uniform_real_distribution<> distribution_x(frame.left_bottom.x, frame.right_top.x);
    std::uniform_real_distribution<> distribution_y(frame.left_bottom.y, frame.right_top.y);
    long union_hits = 0;
    long intersection_hits = 0;
    for (long long i = 0; i < tries; ++i) {
      bool is_inside_all = true;
      bool is_first_hit = true;
      const RealPoint point = {distribution_x(gen), distribution_y(gen)};
      for (std::size_t j = 0; j < ellipses.size(); ++j) {
        if (isInside(point, ellipses[j])) {
          if (is_first_hit) {
            union_hits++;
            is_first_hit = false;
          }
          if (!is_inside_all) {
            break;
          }
        } else {
          is_inside_all = false;
        }
      }
      if (is_inside_all) {
        intersection_hits++;
      }
    }
    return {union_hits, intersection_hits};
  }

  void promiseValueSetter(const std::vector< Ellipse >& ellipses, Rectangle frame, long tries, long long seed,
      std::promise< std::pair< long, long > > p)
  {
    p.set_value(calc(ellipses, frame, tries, seed));
  }

  Rectangle findFrame(const std::vector< Ellipse >& ellipses)
  {
    const Ellipse& first = ellipses[0];
    double right_bound = first.center.x + first.rx;
    double left_bound = first.center.x - first.rx;
    double top_bound = first.center.y + first.ry;
    double bottom_bound = first.center.y - first.ry;
    for (std::size_t i = 0; i < ellipses.size(); ++i) {
      const Ellipse& cur = ellipses[i];
      if (right_bound < cur.center.x + cur.rx) {
        right_bound = cur.center.x + cur.rx;
      }
      if (left_bound > cur.center.x - cur.rx) {
        left_bound = cur.center.x - cur.rx;
      }
      if (top_bound < cur.center.y + cur.ry) {
        top_bound = cur.center.y + cur.ry;
      }
      if (bottom_bound > cur.center.y - cur.ry) {
        bottom_bound = cur.center.y - cur.ry;
      }
    }
    return Rectangle{{left_bound, bottom_bound}, {right_bound, top_bound}};
  }

  double getRectangleArea(Rectangle rectangle)
  {
    return (rectangle.right_top.x - rectangle.left_bottom.x) * (rectangle.right_top.y - rectangle.left_bottom.y);
  }

  std::pair< double, double > getArea(const std::vector< Ellipse >& ellipses, long threads, long tries, long long seed)
  {
    long tries_per_thread = tries / threads;
    Rectangle frame = findFrame(ellipses);
    std::vector< std::thread > descriptors;
    std::vector< std::future< std::pair< long, long > > > futures;
    for (long long i = 0; i < threads - 1; ++i) {
      std::promise< std::pair< long, long > > p;
      futures.push_back(p.get_future());
      descriptors.emplace_back(promiseValueSetter, ellipses, frame, tries_per_thread, seed + i, std::move(p));
    }
    std::promise< std::pair< long, long > > p;
    futures.push_back(p.get_future());
    descriptors.emplace_back(
        promiseValueSetter, ellipses, frame, tries_per_thread + (tries % threads), seed + threads - 1, std::move(p));

    long long total_union = 0;
    long long total_intersection = 0;
    for (std::size_t i = 0; i < futures.size(); ++i) {
      auto res = futures[i].get();
      total_union += res.first;
      total_intersection += res.second;
    }
    for (std::size_t i = 0; i < descriptors.size(); ++i) {
      descriptors[i].join();
    }

    const double area = getRectangleArea(frame);
    return {area * total_union / tries, area * total_intersection / tries};
  }

  constexpr int min_args = 3;
  constexpr int max_args = 4;
  constexpr int base = 10;
  constexpr int threads_arg = 1;
  constexpr int tries_arg = 2;
  constexpr int seed_arg = 3;
}

int main(int argc, char** argv)
{
  if (argc < tarasenko::min_args || argc > tarasenko::max_args) {
    std::cerr << "Incorrect number of arguments\n";
    return 1;
  }
  long threads = std::strtol(argv[tarasenko::threads_arg], nullptr, tarasenko::base);
  if (threads < 0) {
    std::cerr << "Negative threads number\n";
    return 1;
  }
  const long hardware_threads = std::thread::hardware_concurrency();
  threads = std::min(threads, hardware_threads);
  if (threads == 0) {
    threads = 1;
  }
  const long tries = std::strtol(argv[tarasenko::tries_arg], nullptr, tarasenko::base);
  if (tries <= 0) {
    std::cerr << "Non-positive tries number\n";
    return 1;
  }
  long long seed = 0;
  if (argc == tarasenko::max_args) {
    seed = std::strtoll(argv[tarasenko::seed_arg], nullptr, tarasenko::base);
    if (seed < 0) {
      std::cerr << "Negative seed initializing value\n";
      return 1;
    }
  }

  std::vector< tarasenko::Ellipse > ellipses = {};
  tarasenko::Ellipse cur = {};
  long rx = 0, ry = 0, x = 0, y = 0;
  while (std::cin >> rx >> ry >> x >> y) {
    cur.rx = rx;
    cur.ry = ry == 0 ? rx : ry;
    cur.center.x = x;
    cur.center.y = y;
    ellipses.push_back(cur);
  }
  if (!std::cin.eof()) {
    std::cerr << "Incorrect input\n";
    return 1;
  }
  if (ellipses.empty()) {
    std::cout << 0 << ' ' << 0 << '\n';
    return 0;
  }

  auto res = tarasenko::getArea(ellipses, threads, tries, seed);
  std::cout << std::setprecision(std::numeric_limits< double >::max_digits10);
  std::cout << res.first << ' ' << res.second << '\n';
}
