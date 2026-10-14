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
  const int minArgs = 3;
  const int maxArgs = 4;
  const int threadsIndex = 1;
  const int triesIndex = 2;
  const int seedIndex = 3;

  if ((argc < minArgs) || (argc > maxArgs))
  {
    std::cerr << "Wrong number of arguments\n";
    return 1;
  }

  try
  {
    size_t threadCount = studilova::parseArgument(argv[threadsIndex]);
    size_t tries = studilova::parseArgument(argv[triesIndex]);

    size_t seed = 0;
    if (argc == maxArgs)
    {
      seed = studilova::parseArgument(argv[seedIndex]);
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

      const studilova::Circle circle =
      {
        static_cast< double >(radius),
        static_cast< double >(x),
        static_cast< double >(y)
      };

      circles.push_back(circle);
    }

    const auto areas = studilova::calculateAreas(circles, threadCount, tries, seed);
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
