#ifndef CALCULATION_HPP
#define CALCULATION_HPP

#include <random>
#include <vector>
#include "shape.hpp"

namespace kondrat
{
  double calculateArea(
    const std::vector< Shape > & shapes,
    long long threads,
    long long tries,
    std::default_random_engine & generator);
}

#endif
