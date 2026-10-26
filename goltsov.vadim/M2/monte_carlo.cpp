#include "monte_carlo.hpp"
#include <future>
#include <random>

bool goltsov::isInside(Circle* s, const double& x, const double& y)
{
  const double r = s->r;
  const double sx = s->x;
  const double sy = s->y;
  return r * r >= (x - sx) * (x - sx) + (y - sy) * (y - sy);
}

size_t goltsov::calc(const std::vector< Circle* >& data, const size_t& tests, const size_t& seed, const double& min_x,
    const double& min_y, const double& max_x, const double& max_y)
{
  std::default_random_engine engine(seed);
  std::uniform_real_distribution< double > dist_x(min_x, max_x);
  std::uniform_real_distribution< double > dist_y(min_y, max_y);

  size_t count_inside = 0;

  for (size_t i = 0; i < tests; ++i)
  {
    const double x = dist_x(engine);
    const double y = dist_y(engine);

    for (size_t j = 0; j < data.size(); ++j)
    {
      if (isInside(data[j], x, y))
      {
        count_inside++;
        break;
      }
    }
  }
  return count_inside;
}

double goltsov::area(const std::vector< Circle* >& data, const size_t& threads, const size_t& tries,
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
