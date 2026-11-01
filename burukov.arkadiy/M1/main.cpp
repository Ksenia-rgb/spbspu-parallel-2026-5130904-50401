#include "shapes.hpp"

#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

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
    std::cerr << "Usage: ./lab threads tries [seed]" << '\n';
    return bad_exit;
  }

  long long threads = 0;
  long long tries = 0;
  long long seed = 0;

  try
  {
    threads = burukov::parseArgument(argv[threads_idx], "threads");
    tries = burukov::parseArgument(argv[tries_idx], "tries");
    if (argc == max_args)
    {
      seed = burukov::parseArgument(argv[seed_idx], "seed");
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
    std::cerr << "threads and seed must be non-negative, tries must be positive" << '\n';
    return bad_exit;
  }

  threads = (threads > 0) ? threads : 1;

  std::vector< std::unique_ptr< burukov::Shape > > shapes;

  double r = 0.0;
  double second = 0.0;
  double x = 0.0;
  double y = 0.0;

  while (std::cin >> r)
  {
    if (!(std::cin >> second >> x >> y))
    {
      std::cerr << "Error: cannot parse figure parameters" << '\n';
      return bad_exit;
    }

    if (r <= 0.0)
    {
      std::cerr << "Radius must be positive" << '\n';
      return bad_exit;
    }

    if (second == 0.0)
    {
      shapes.push_back(std::make_unique< burukov::Circle >(r, x, y));
    }
    else
    {
      const double b = (second < 0.0) ? -second : second;
      shapes.push_back(std::make_unique< burukov::Ellipse >(r, b, x, y));
    }
  }

  if (shapes.empty())
  {
    std::cout << "0 0" << '\n';
    return good_exit;
  }

  const burukov::rectangle_t rect = burukov::findRectangleRange(shapes);

  try
  {
    const auto result = burukov::area(threads, tries, seed, shapes, rect);
    const double area_union = result.first;
    const double area_intersect = result.second;

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
