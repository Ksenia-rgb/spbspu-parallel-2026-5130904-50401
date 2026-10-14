#include <iostream>

namespace donkeev {
  struct Point {
    double x;
    double y;
  };

  struct Circle {
    double radius;
    Point center;
  };

  bool contains(const Circle&, const Point&);
}

int main(int argc, char **argv)
{
  if (argc < 3 || argc > 4) {
    std::cerr << "Invalid arguments\n";
    return 1;
  }

  long long threads = 0, tries = 0, seed = 0;
  try {
    threads = std::stoll(argv[1]);
    tries = std::stoll(argv[2]);
    seed = (argc > 3) ? std::stoi(argv[3]) : 0;
  } catch (const std::exception& e) {
    std::cerr << "Arguments error " << e.what() << std::endl;
    return 1;
  }

  if (threads < 0 || tries <= 0 || seed < 0) {
    std::cerr << "Threads, tries and seed must be positive\n";
    return 1;
  }
  threads = threads > 0 ? threads : 1;


}

bool donkeev::contains(const Circle& c, const Point& p) {
  double dx = p.x - c.center.x;
  double dy = p.y - c.center.y;
  return dx * dx + dy * dy <= c.radius * c.radius;
}
