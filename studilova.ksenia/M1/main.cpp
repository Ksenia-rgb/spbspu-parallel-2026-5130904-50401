#include <iostream>
#include <cstddef>
#include <exception>
#include <vector>
#include <stdexcept>

#include "calculations.hpp"

int main()
{
  size_t threadCount = 4;
  size_t tries = 1000000;
  size_t seed = 42;

  try
  {
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
    std::cout << areas.first << ' ' << areas.second << '\n';
  }
  catch (const std::exception& error)
  {
    std::cerr << error.what() << '\n';
    return 1;
  }

  return 0;
}
