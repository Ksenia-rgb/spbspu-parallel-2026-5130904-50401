#include "monte_carlo.hpp"
#include <future>
#include <random>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

bool goltsov::isInside(const Circle* s, const double& x, const double& y)
{
  const double r = s->r;
  const double sx = s->x;
  const double sy = s->y;
  return r * r >= (x - sx) * (x - sx) + (y - sy) * (y - sy);
}

size_t goltsov::calc(const std::vector< const Circle* >& data, const size_t& tests, const size_t& seed,
    const double& min_x, const double& min_y, const double& max_x, const double& max_y)
{
  std::mt19937_64 engine(seed);
  const int mantissa_bits = std::numeric_limits< double >::digits;
  const int shift = std::numeric_limits< std::uint64_t >::digits - mantissa_bits;
  const double scale = 1.0 / static_cast< double >(1ULL << mantissa_bits);

  size_t count_inside = 0;

  for (size_t i = 0; i < tests; ++i)
  {
    const double x = min_x + (max_x - min_x) * static_cast< double >(engine() >> shift) * scale;
    const double y = min_y + (max_y - min_y) * static_cast< double >(engine() >> shift) * scale;

    for (size_t j = 0; j < data.size(); ++j)
    {
      if (isInside(data.at(j), x, y))
      {
        count_inside++;
        break;
      }
    }
  }
  return count_inside;
}

double goltsov::area(const std::vector< const Circle* >& data, const size_t& threads, const size_t& tries,
    const size_t& start_seed, const double& min_x, const double& min_y, const double& max_x, const double& max_y)
{
  std::vector< std::future< size_t > > results_in_threads(threads);

  for (size_t i = 0; i < threads; ++i)
  {
    size_t tests = tries / threads + (i < tries % threads ? 1 : 0);
    results_in_threads[i]
        = std::async(std::launch::async, calc, data, tests, start_seed + i, min_x, min_y, max_x, max_y);
  }

  size_t count_inside = 0;

  for (size_t i = 0; i < threads; ++i)
  {
    count_inside += results_in_threads[i].get();
  }

  return (max_x - min_x) * (max_y - min_y) * (static_cast< double >(count_inside) / static_cast< double >(tries));
}
