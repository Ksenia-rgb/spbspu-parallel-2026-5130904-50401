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

  struct frame_t
  {
    double min_x = 0;
    double max_x = 0;
    double min_y = 0;
    double max_y = 0;
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

  frame_t computeFrame(const std::vector< circle_t > & circles)
  {
    frame_t frame;
    frame.min_x = circles[0].cx - circles[0].r;
    frame.max_x = circles[0].cx + circles[0].r;
    frame.min_y = circles[0].cy - circles[0].r;
    frame.max_y = circles[0].cy + circles[0].r;
    double min_x = 0;
    double max_x = 0;
    double min_y = 0;
    double max_y = 0;
    for (size_t i = 1; i < circles.size(); ++i) {
      min_x = circles[i].cx - circles[i].r;
      max_x = circles[i].cx + circles[i].r;
      min_y = circles[i].cy - circles[i].r;
      max_y = circles[i].cy + circles[i].r;
      if (frame.min_x > min_x) {
        frame.min_x = min_x;
      }
      if (frame.max_x < max_x) {
        frame.max_x = max_x;
      }
      if (frame.min_y > min_y) {
        frame.min_y = min_y;
      }
      if (frame.max_y < max_y) {
        frame.max_y = max_y;
      }
    }
    return frame;
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

  madieva::frame_t frame = madieva::computeFrame(circles);

  return 0;
}
