#include <cstddef>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

#include "area.hpp"
#include "input.hpp"

int main(int argc, char **argv)
{
  const int short_form = 3;
  const int full_form = 4;
  if ((argc != short_form) && (argc != full_form)) {
    std::cerr << "Usage: lab threads tries [seed]\n";
    return 1;
  }

  const int threads_pos = 1;
  const int tries_pos = 2;
  const int seed_pos = 3;
  std::size_t threads = 0;
  std::size_t tries = 0;
  std::size_t seed = 0;
  try {
    threads = petrov::parseCount(argv[threads_pos]);
    tries = petrov::parseCount(argv[tries_pos]);
    if (argc == full_form) {
      seed = petrov::parseCount(argv[seed_pos]);
    }
  } catch (const std::exception &e) {
    std::cerr << "Bad argument: " << e.what() << '\n';
    return 1;
  }
  if (tries == 0) {
    std::cerr << "Number of tries must be positive\n";
    return 1;
  }

  std::vector< petrov::circle_t > circles;
  try {
    circles = petrov::readCircles(std::cin);
  } catch (const std::invalid_argument &e) {
    std::cerr << "Bad input: " << e.what() << '\n';
    return 1;
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 2;
  }

  try {
    const petrov::areas_t areas = petrov::estimateAreas(circles, threads, tries, seed);
    std::cout << std::setprecision(std::numeric_limits< double >::max_digits10);
    std::cout << areas.covered << ' ' << areas.common << '\n';
  } catch (const std::exception &e) {
    std::cerr << "Calculation failed: " << e.what() << '\n';
    return 2;
  }
  return 0;
}
