#include <iostream>
#include <algorithm>
#include <cstddef>
#include <future>
#include <iomanip>
#include <limits>
#include <random>
#include <vector>
#include <tuple>
#include <utility>

namespace lukashevich
{
  bool isInside(const std::tuple< double, double, double >& circle, const double& x, const double& y);

  std::pair< size_t, size_t > calc(const std::vector< std::tuple< double, double, double > > & data,
    std::size_t tests, std::size_t seed, double min_x, double min_y,
    double max_x, double max_y);

  std::pair< double, double > area(const std::vector< std::tuple< double, double, double > >& data,
    const size_t& threads, const size_t& tries, const size_t& start_seed, const double& min_x, const double& min_y,
    const double& max_x, const double& max_y);
}

bool lukashevich::isInside(const std::tuple< double, double, double >& s, const double& x, const double& y)
{
  const double r = std::get< 0 >(s);
  const double sx = std::get< 1 >(s);
  const double sy = std::get< 2 >(s);
  return r * r >= (x - sx) * (x - sx) + (y - sy) * (y - sy);
}

double min_x = std::numeric_limits< double >::max();
double min_y = std::numeric_limits< double >::max();
double max_x = std::numeric_limits< double >::min();
double max_y = std::numeric_limits< double >::min();

std::pair< size_t, size_t > lukashevich::calc(const std::vector< std::tuple< double, double, double > > & data,
    std::size_t tests, std::size_t seed, double min_x, double min_y,
    double max_x, double max_y)
{
  std::default_random_engine engine(seed);
  std::uniform_real_distribution< double > dist_x(min_x, max_x);
  std::uniform_real_distribution< double > dist_y(min_y, max_y);

  size_t count_inside = 0;
  size_t count_inside_all = 0;

  for (size_t i = 0; i < tests; ++i)
  {
    const double x = dist_x(engine);
    const double y = dist_y(engine);
    bool is_inside = false;
    bool is_inside_all = true;

    for (size_t j = 0; j < data.size(); ++j)
    {
      if (lukashevich::isInside(data[j], x, y))
      {
        is_inside = true;
      }
      else
      {
        is_inside_all = false;
      }
    }

    if (is_inside)
    {
      count_inside++;
    }
    if (is_inside_all)
    {
      count_inside_all++;
    }
  }
  return std::pair< size_t, size_t >(count_inside, count_inside_all);
}

std::pair< double, double > lukashevich::area(const std::vector< std::tuple< double, double, double > >& data,
    const size_t& threads, const size_t& tries, const size_t& start_seed, const double& min_x, const double& min_y,
    const double& max_x, const double& max_y)
{
  std::vector< std::future< std::pair< size_t, size_t > > > results_in_threads(threads);

  for (size_t i = 0; i < tries % threads; ++i)
  {
    results_in_threads[i]
        = std::async(std::launch::async, calc, data, tries / threads + 1, start_seed + i, min_x, min_y, max_x, max_y);
  }
  for (size_t i = tries % threads; i < threads; ++i)
  {
    results_in_threads[i]
        = std::async(std::launch::async, calc, data, tries / threads, start_seed + i, min_x, min_y, max_x, max_y);
  }

  size_t count_inside = 0;
  size_t count_inside_all = 0;

  for (size_t i = 0; i < threads; ++i)
  {
    const std::pair< size_t, size_t > result_f = results_in_threads[i].get();
    count_inside += result_f.first;
    count_inside_all += result_f.second;
  }

  return std::pair< double, double >(
      (max_x - min_x) * (max_y - min_y) * (static_cast< double >(count_inside) / static_cast< double >(tries)),
      (max_x - min_x) * (max_y - min_y) * (static_cast< double >(count_inside_all) / static_cast< double >(tries)));
}

int main(int argc, char** argv)
{
  if (argc != 3 && argc != 4)
  {
    std::cerr << "Invalid count command line arguments\n";
    return 1;
  }

}
