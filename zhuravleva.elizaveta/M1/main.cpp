#include "monteCarlo.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <iomanip>
#include <limits>
#include <cstddef>

int main()
{
  try
  {
    const std::vector< zhuravleva::Circle > circles = zhuravleva::readCircles(std::cin);
    if (circles.empty())
    {
      std::cout << "0 0\n";
      return 0;
    }
    const zhuravleva::Bounds bounds = zhuravleva::getBounds(circles);
    const double width = bounds.maxX - bounds.minX;
    const double height = bounds.maxY - bounds.minY;
    const double area = width * height;

    const std::size_t tries = 1000000;
    const zhuravleva::Hits hits = zhuravleva::calcMonte(circles, bounds, tries, 0);
    const double areaAny = area * static_cast< double >(hits.any) / tries;
    const double areaAll = area * static_cast< double >(hits.all) / tries;

    std::cout << std::setprecision(std::numeric_limits< double >::max_digits10);
    std::cout << areaAny << ' ' << areaAll << '\n';
  }
  catch (const std::invalid_argument& e)
  {
    std::cerr << e.what() << '\n';
    return 1;
  }
  return 0;
}
