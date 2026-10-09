#include <iostream>
#include <vector>

namespace chernov {
  struct Circle {
    int x, y, radius;
  };

  void inputCircles(std::vector< Circle > &circles)
  {
    int radius = 0, _ = 0, x = 0, y = 0;
    while (std::cin >> radius >> _ >> x >> y) {
      circles.push_back(Circle{x, y, radius});
    }

    if (!std::cin.eof()) {
      throw std::runtime_error("input error");
    }
  }
}

int main(int argc, char **argv)
{
  using namespace chernov;

  if (argc < 3 || argc > 4) {
    std::cerr << "count of arguments is incorrect\n";
    return 1;
  }

  long long threads = 0, tries = 0, seed = 0;

  try {
    threads = std::stoll(argv[1]);
  } catch (const std::invalid_argument &e) {
    std::cerr << "count of threads is incorrect\n";
    return 1;
  }
  if (threads < 0) {
    std::cerr << "count of threads is incorrect\n";
    return 1;
  }
  if (threads == 0) {
    threads = 1;
  }

  try {
    tries = std::stoll(argv[2]);
  } catch (const std::invalid_argument &e) {
    std::cerr << "count of tries is incorrect\n";
    return 1;
  }
  if (tries < 1) {
    std::cerr << "count of tries is incorrect\n";
    return 1;
  }

  if (argc == 4) {
    try {
      seed = std::stoll(argv[3]);
    } catch (const std::invalid_argument &e) {
      std::cerr << "seed is incorrect\n";
      return 1;
    }
    if (seed < 0) {
      std::cerr << "seed is incorrect\n";
      return 1;
    }
  }

  std::vector< Circle > circles;
  try {
    inputCircles(circles);
  } catch (const std::runtime_error &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
