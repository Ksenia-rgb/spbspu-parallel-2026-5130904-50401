#include <iostream>
#include <cstddef>
#include <exception>
#include <vector>
#include <stdexcept>
#include <iomanip>
#include <limits>

#include "calculations.hpp"

int main(int argc, char** argv)
{
  const int min_args = 3;
  const int max_args = 4;
  const int threads_index = 1;
  const int tries_index = 2;
  const int seed_index = 3;

  if ((argc < min_args) || (argc > max_args))
  {
    std::cerr << "Wrong number of arguments\n";
    return 1;
  }

  try
  {
    const size_t thread_count = studilova::parseArgument(argv[threads_index]);
    const size_t tries = studilova::parseArgument(argv[tries_index]);

    size_t seed = 0;
    if (argc == max_args)
    {
      seed = studilova::parseArgument(argv[seed_index]);
    }

    if (tries == 0)
    {
      throw std::invalid_argument("Number of tries must be positive");
    }

    std::vector< studilova::Circle > circles;

    while (true)
    {
      std::cin >> std::ws;

      if (std::cin.bad())
      {
        throw std::invalid_argument("Input error");
      }

      if (std::cin.eof())
      {
        break;
      }

      int radius = 0;
      int ignored = 0;
      int x = 0;
      int y = 0;

      if (!(std::cin >> radius >> ignored >> x >> y))
      {
        throw std::invalid_argument("Cannot read circle parameters");
      }

      const studilova::Circle circle
          = {static_cast< double >(radius), static_cast< double >(x), static_cast< double >(y)};

      circles.push_back(circle);
    }

    const auto areas = studilova::calculateAreas(circles, thread_count, tries, seed);
    std::cout << std::setprecision(std::numeric_limits< double >::max_digits10);
    std::cout << areas.first << ' ' << areas.second << '\n';
  }
  catch (const std::invalid_argument& error)
  {
    std::cerr << error.what() << '\n';
    return 1;
  }
  catch (const std::out_of_range& error)
  {
    std::cerr << error.what() << '\n';
    return 1;
  }
  catch (const std::exception& error)
  {
    std::cerr << error.what() << '\n';
    return 2;
  }

  return 0;
}
