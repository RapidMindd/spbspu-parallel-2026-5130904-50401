#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <thread>
#include <vector>
#include "../common/area.hpp"

namespace tarasenko
{
  constexpr int min_args = 3;
  constexpr int max_args = 4;
  constexpr int base = 10;
  constexpr int threads_arg = 1;
  constexpr int tries_arg = 2;
  constexpr int seed_arg = 3;
}

int main(int argc, char** argv)
{
  if (argc < tarasenko::min_args || argc > tarasenko::max_args)
  {
    std::cerr << "Incorrect number of arguments\n";
    return 1;
  }
  long threads = std::strtol(argv[tarasenko::threads_arg], nullptr, tarasenko::base);
  if (threads < 0)
  {
    std::cerr << "Negative threads number\n";
    return 1;
  }
  const long hardware_threads = std::thread::hardware_concurrency();
  threads = std::min(threads, hardware_threads);
  if (threads == 0)
  {
    threads = 1;
  }
  const long tries = std::strtol(argv[tarasenko::tries_arg], nullptr, tarasenko::base);
  if (tries <= 0)
  {
    std::cerr << "Non-positive tries number\n";
    return 1;
  }
  long long seed = 0;
  if (argc == tarasenko::max_args)
  {
    seed = std::strtoll(argv[tarasenko::seed_arg], nullptr, tarasenko::base);
    if (seed < 0)
    {
      std::cerr << "Negative seed initializing value\n";
      return 1;
    }
  }

  std::vector< tarasenko::Ellipse > ellipses = {};
  tarasenko::Ellipse cur = {};
  long rx = 0, ry = 0, x = 0, y = 0;
  while (std::cin >> rx >> ry >> x >> y)
  {
    cur.rx = rx;
    cur.ry = ry == 0 ? rx : ry;
    cur.center.x = x;
    cur.center.y = y;
    ellipses.push_back(cur);
  }
  if (!std::cin.eof())
  {
    std::cerr << "Incorrect input\n";
    return 1;
  }
  if (ellipses.empty())
  {
    std::cout << 0 << ' ' << 0 << '\n';
    return 0;
  }

  auto res = tarasenko::getArea(ellipses.data(), ellipses.size(), threads, tries, seed);
  std::cout << std::setprecision(std::numeric_limits< double >::max_digits10);
  std::cout << res.first << ' ' << res.second << '\n';
}
