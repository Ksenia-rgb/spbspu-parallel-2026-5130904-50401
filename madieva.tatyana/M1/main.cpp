#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace madieva {

  struct circle_t
  {
    double r;
    double cx;
    double cy;
  };

  struct params_t
  {
    std::size_t threads = 0;
    std::size_t tries = 0;
    std::size_t seed = 0;
  };

  bool parseParams(int argc, char ** argv, params_t & params)
  {
    if (argc != 3 && argc != 4) {
      std::cerr << "incorrect number of arguments\n";
      return false;
    }
    try {
      params.threads = static_cast< size_t >(std::stoull(argv[1]));
      params.tries = static_cast< size_t >(std::stoull(argv[2]));
      if (argc == 4) {
        params.seed = static_cast< size_t >(std::stoull(argv[3]));
      }
    } catch (const std::exception & e) {
      std::cerr << "cannot parse arguments: " << e.what() << "\n";
      return false;
    }
    if (params.threads == 0) {
      std::cerr << "inappropriate number of threads\n";
      return false;
    }
    if (params.tries == 0) {
      std::cerr << "incorrect number of tries\n";
      return false;
    }
    return true;
  }

  bool readCircles(std::vector< circle_t > & circles)
  {
    double r = 0;
    double index = 0;
    double cx = 0;
    double cy = 0;

    while (std::cin >> r >> index >> cx >> cy) {
      circles.push_back({r, cx, cy});
    }
    if (!std::cin.eof()) {
      std::cerr << "error reading figures\n";
      return false;
    }
    if (circles.empty()) {
      std::cerr << "no figures were provided\n";
      return false;
    }
    return true;
  }

}

int main(int argc, char ** argv)
{
  madieva::params_t params;
  if (!madieva::parseParams(argc, argv, params)){
    return 1;
  }

  std::vector< madieva::circle_t > circles;
  if (!madieva::readCircles(circles)) {
    return 1;
  }

  return 0;
}
