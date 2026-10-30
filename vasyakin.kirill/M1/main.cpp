#include "utils.hpp"

#include <iostream>
#include <iomanip>
#include <limits>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <memory>

int main(int argc, char** argv)
{
  constexpr int min_args = 3;
  constexpr int max_args = 4;

  constexpr int threads_idx = 1;
  constexpr int tries_idx = 2;
  constexpr int seed_idx = 3;

  constexpr int bad_exit = 1;
  constexpr int good_exit = 0;

  if (argc < min_args || argc > max_args)
  {
    std::cerr << "Too much" << '\n';
    return bad_exit;
  }

  long long threads = 0, tries = 0, seed = 0;

  try
  {
    threads = vasyakin::parseArgument(argv[threads_idx], "threads");
    tries = vasyakin::parseArgument(argv[tries_idx], "tries");

    if (argc == max_args)
    {
      seed = vasyakin::parseArgument(argv[seed_idx], "seed");
    }
  }
  catch (const std::invalid_argument& e)
  {
    std::cerr << e.what() << '\n';
    return bad_exit;
  }
  catch (const std::out_of_range& e)
  {
    std::cerr << "Number is out of range: " << e.what() << '\n';
    return bad_exit;
  }

  if (threads < 0 || tries <= 0 || seed < 0)
  {
    std::cerr << "threads, tries and seed must be positive" << '\n';
    return bad_exit;
  }

  threads = threads > 0 ? threads : 1;

  std::vector< std::unique_ptr< vasyakin::Figure > > figures;

  int r = 0, dop_task_param = 0, x = 0, y = 0;
  while (std::cin >> r >> dop_task_param >> x >> y)
  {
    if (dop_task_param == 0)
    {
      figures.push_back(std::make_unique< vasyakin::Circle >(r, x, y));
    }
    else
    {
      figures.push_back(std::make_unique< vasyakin::Ellipse >(r, dop_task_param, x, y));
    }
  }

  if (!std::cin.eof())
  {
    std::cerr << "Error: cannot parse figure parameters" << '\n';
    return bad_exit;
  }

  if (figures.empty())
  {
    std::cerr << "Vector of figures is empty" << '\n';
    return bad_exit;
  }

  const vasyakin::rectangle_t rect = vasyakin::findRectangleRange(figures);

  try
  {
    constexpr long long max_allowed_threads = 1000;
    const long long actual_threads = std::min(threads, max_allowed_threads);

    const auto pair = vasyakin::area(actual_threads, tries, seed, figures, rect);

    const double area_union = pair.first;
    const double area_intersect = pair.second;

    std::cout << std::setprecision(std::numeric_limits< double >::max_digits10);
    std::cout << area_union << " " << area_intersect << '\n';
  }
  catch (const std::invalid_argument& e)
  {
    std::cerr << e.what() << '\n';
    return bad_exit;
  }

  return good_exit;
}
