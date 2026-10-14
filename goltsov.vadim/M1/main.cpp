#include <iostream>
#include <vector>
#include <tuple>
#include <random>
#include <future>
#include <utility>
#include <cstddef>
#include <string>
#include <limits>
#include <algorithm>
#include <stdexcept>

namespace goltsov
{
  bool isInside(const std::tuple< double, double, double >& s, const double& x, const double& y);

  std::pair< size_t, size_t > calc(const std::vector< std::tuple< double, double, double > >& data, const size_t& tests,
      const size_t& seed, const double& min_x, const double& min_y, const double& max_x, const double& max_y);

  std::pair< double, double > areas(const std::vector< std::tuple< double, double, double > >& data,
      const size_t& threads, const size_t& tries, const size_t& start_seed, const double& min_x, const double& min_y,
      const double& max_x, const double& max_y);
}

int main(int argc, char** argv)
{
  if (argc != 3 && argc != 4)
  {
    std::cerr << "Invalid count command line arguments\n";
    return 1;
  }

  if (argv[1][0] == '-')
  {
    std::cerr << "Invalid threads argument. Threads must be a not negative number\n";
    return 1;
  }
  size_t threads = 0;
  try
  {
    threads = std::stoull(argv[1]);
  }
  catch (std::invalid_argument& e)
  {
    std::cerr << e.what();
    return 2;
  }

  if (argv[2][0] == '-')
  {
    std::cerr << "Invalid tries argument. Tries must be a positive number\n";
    return 1;
  }
  size_t tries = 0;
  try
  {
    tries = std::stoull(argv[2]);
  }
  catch (std::invalid_argument& e)
  {
    std::cerr << e.what();
    return 2;
  }
  if (tries == 0)
  {
    std::cerr << "Invalid tries argument. Tries must be a positive number\n";
    return 1;
  }

  size_t seed = 0;
  if (argc == 4)
  {
    if (argv[3][0] == '-')
    {
      std::cerr << "Invalid seed argument. Seed must be a positive number\n";
      return 1;
    }
    try
    {
      seed = std::stoull(argv[3]);
    }
    catch (std::invalid_argument& e)
    {
      std::cerr << e.what();
      return 2;
    }
  }

  double min_x = std::numeric_limits< double >::max();
  double min_y = std::numeric_limits< double >::max();
  double max_x = std::numeric_limits< double >::min();
  double max_y = std::numeric_limits< double >::min();

  std::vector< std::tuple< double, double, double > > data;
  while (!std::cin.eof())
  {
    double r = 0.0;
    if (!(std::cin >> r))
    {
      if (std::cin.eof())
      {
        break;
      }
      std::cerr << "Invalid data\n";
      return 1;
    }
    double ignore = 0.0;
    if (!(std::cin >> ignore))
    {
      std::cerr << "Invalid data\n";
      return 1;
    }
    double x = 0.0;
    if (!(std::cin >> x))
    {
      std::cerr << "Invalid data\n";
      return 1;
    }
    double y = 0.0;
    if (!(std::cin >> y))
    {
      std::cerr << "Invalid data\n";
      return 1;
    }
    min_x = std::min(min_x, x - r);
    min_y = std::min(min_y, y - r);
    max_x = std::max(max_x, x + r);
    max_y = std::max(max_y, y + r);
    data.push_back(std::tuple< double, double, double >(r, x, y));
  }

  const std::pair< double, double > areas
      = goltsov::areas(data, threads != 0 ? threads : 1, tries, seed, min_x, min_y, max_x, max_y);
  std::cout << areas.first << " " << areas.second << "\n";
}

bool goltsov::isInside(const std::tuple< double, double, double >& s, const double& x, const double& y)
{
  const double r = std::get< 0 >(s);
  const double sx = std::get< 1 >(s);
  const double sy = std::get< 2 >(s);
  return r * r >= (x - sx) * (x - sx) + (y - sy) * (y - sy);
}

std::pair< size_t, size_t > goltsov::calc(const std::vector< std::tuple< double, double, double > >& data,
    const size_t& tests, const size_t& seed, const double& min_x, const double& min_y, const double& max_x,
    const double& max_y)
{
  std::default_random_engine engine(seed);
  std::uniform_real_distribution< double > dist_x(min_x, max_x);
  std::uniform_real_distribution< double > dist_y(min_y, max_y);

  size_t count_inside = 0;
  size_t count_inside_in_all = 0;

  for (size_t i = 0; i < tests; ++i)
  {
    const double x = dist_x(engine);
    const double y = dist_y(engine);
    bool is_inside = false;
    bool is_inside_in_all = true;

    for (size_t j = 0; j < data.size(); ++j)
    {
      if (isInside(data[j], x, y))
      {
        is_inside = true;
      }
      else
      {
        is_inside_in_all = false;
      }
    }

    if (is_inside)
    {
      count_inside++;
    }
    if (is_inside_in_all)
    {
      count_inside_in_all++;
    }
  }
  return std::pair< size_t, size_t >(count_inside, count_inside_in_all);
}

std::pair< double, double > goltsov::areas(const std::vector< std::tuple< double, double, double > >& data,
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
  size_t count_inside_in_all = 0;

  for (size_t i = 0; i < threads; ++i)
  {
    const std::pair< size_t, size_t > result_i = results_in_threads[i].get();
    count_inside += result_i.first;
    count_inside_in_all += result_i.second;
  }

  return std::pair< double, double >(
      (max_x - min_x) * (max_y - min_y) * (static_cast< double >(count_inside) / static_cast< double >(tries)),
      (max_x - min_x) * (max_y - min_y) * (static_cast< double >(count_inside_in_all) / static_cast< double >(tries)));
}
