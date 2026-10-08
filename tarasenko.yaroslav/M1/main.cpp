#include <future>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>

struct Point
{
  long x;
  long y;
};

struct RealPoint
{
  double x;
  double y;
};

struct Circle
{
  long radius;
  Point center;
};

struct Rectangle
{
  Point left_bottom;
  Point right_top;
};

bool isInside(RealPoint point, Circle circle)
{
  point.x -= circle.center.x;
  point.y -= circle.center.y;
  return point.x * point.x + point.y * point.y <= circle.radius * circle.radius;
}

std::pair< long, long > calc(const std::vector< Circle >& circles, Rectangle frame, long tries,
                             long long seed)
{
  std::mt19937 gen(seed);
  std::uniform_real_distribution<> distribution_x(frame.left_bottom.x, frame.right_top.x);
  std::uniform_real_distribution<> distribution_y(frame.left_bottom.y, frame.right_top.y);
  long unionHits = 0;
  long intersectionHits = 0;
  for (long long i = 0; i < tries; ++i) {
    bool isInsideAll = true;
    bool isFirstHit = true;
    RealPoint point = {distribution_x(gen), distribution_y(gen)}  ;
    for (size_t j = 0; j < circles.size(); ++j) {
      if (isInside(point, circles[j])) {
        if (isFirstHit) {
          unionHits++;
          isFirstHit = false;
        }
        if (!isInsideAll) {
          break;
        }
      }
      else {
        isInsideAll = false;
      }
    }
    if (isInsideAll) {
      intersectionHits++;
    }
  }
  return {unionHits, intersectionHits};
}

void promiseValueSetter(const std::vector< Circle >& circles, Rectangle frame, long tries,
                        long long seed, std::promise< std::pair< long, long > > p)
{
  p.set_value(calc(circles, frame, tries, seed));
}

Rectangle findFrame(const std::vector< Circle >& circles)
{
  const Circle& first = circles[0];
  long right_bound = first.center.x + first.radius;
  long left_bound = first.center.x - first.radius;
  long top_bound = first.center.y + first.radius;
  long bottom_bound = first.center.y - first.radius;
  for (size_t i = 0; i < circles.size(); ++i) {
    const Circle& cur = circles[i];
    if (right_bound < cur.center.x + cur.radius) {
      right_bound = cur.center.x + cur.radius;
    }
    if (left_bound > cur.center.x - cur.radius) {
      left_bound = cur.center.x - cur.radius;
    }
    if (top_bound < cur.center.y + cur.radius) {
      top_bound = cur.center.y + cur.radius;
    }
    if (bottom_bound > cur.center.y - cur.radius) {
      bottom_bound = cur.center.y - cur.radius;
    }
  }
  return Rectangle{{left_bound, bottom_bound}, {right_bound, top_bound}};
}

long long getRectangleArea(Rectangle rectangle)
{
  return (rectangle.right_top.x - rectangle.left_bottom.x) * (rectangle.right_top.y - rectangle.left_bottom.y);
}

std::pair< double, double > getArea(const std::vector< Circle >& circles, long threads, long tries,
                                    long long seed)
{
  long tries_per_thread = threads / tries;
  Rectangle frame = findFrame(circles);
  std::vector< std::thread > descriptors(threads);
  std::vector< std::future< std::pair< long, long > > > futures;
  for (long long i = 0; i < threads - 1; ++i) {
    std::promise< std::pair< long, long > > p;
    futures.push_back(p.get_future());
    descriptors.emplace_back(promiseValueSetter, circles, frame, tries_per_thread, seed + i,
                             std::move(p));
  }
  std::promise< std::pair< long, long > > p;
  futures.push_back(p.get_future());
  descriptors.emplace_back(promiseValueSetter, circles, frame,
                           tries_per_thread + (tries % tries_per_thread), seed + threads,
                           std::move(p));

  long long total_union = 0;
  long long total_intersection = 0;
  for (size_t i = 0; i < futures.size(); ++i) {
    auto res = futures[i].get();
    total_union += res.first;
    total_intersection += res.second;
  }
  for (size_t i = 0; i < descriptors.size(); ++i) {
    descriptors[i].join();
  }

  return {total_union / tries * getRectangleArea(frame), total_intersection / tries * getRectangleArea(frame)};
}

int main(int argc, char** argv)
{
  if (argc < 3 || argc > 4) {
    std::cerr << "Incorrect number of arguments\n";
    return 1;
  }
  long threads = strtol(argv[1], nullptr, 10);
  if (threads < 0) {
    std::cerr << "Negative threads number\n";
    return 1;
  }
  long tries = strtol(argv[2], nullptr, 10);
  if (tries <= 0) {
    std::cerr << "Non-positive tries number\n";
    return 1;
  }
  long long seed = 0;
  if (argc == 4) {
    seed = strtoll(argv[3], nullptr, 10);
    if (seed < 0) {
      std::cerr << "Negative seed initializing value";
      return 1;
    }
  }

  std::vector< Circle > circles = {};
  Circle cur;
  int unused = 0;
  while (std::cin >> cur.radius >> unused >> cur.center.x >> cur.center.y) {
    circles.push_back(cur);
  }
  if (!std::cin.eof()) {
    std::cerr << "Incorrect input\n";
    return 1;
  }

  auto res = getArea(circles, threads, tries, seed);
  std::cout << std::setprecision(std::numeric_limits<double>::max_digits10);
  std::cout << res.first << ' ' << res.second << '\n';
}
