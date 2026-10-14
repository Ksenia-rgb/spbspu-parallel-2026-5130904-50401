#include <iostream>
#include <vector>

namespace donkeev {
  struct Point {
    int x;
    int y;
  };

  struct Circle {
    int radius;
    Point center;
  };

  bool contains(const Circle&, const Point&);
  bool read_circles(std::vector<Circle>&);
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

bool donkeev::read_circles(std::vector<Circle>& out) {
  while (true) {
    int r, ignore, x, y;
    if (std::cin >> r >> ignore >> x >> y) {
        out.push_back({r, {x, y}});
    } else if (std::cin.eof()) {
        break;
    } else {
        std::cerr << "Invalid circle parametrs\n";
        return false;
    }
  }
  
  if (out.empty()) {
      std::cerr << "No circles\n";
      return false;
  }

  return true;
}
