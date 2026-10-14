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

  struct Bounds {
    double min_x, max_x, min_y, max_y;
  };

  bool contains(const Circle&, const Point&);
  bool read_circles(std::vector<Circle>&);
  Bounds compute_bounds(const std::vector<Circle>& circles);
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

  std::vector<donkeev::Circle> circles;
  if (!donkeev::read_circles(circles)) return 1;
    
  donkeev::Bounds bounds = donkeev::compute_bounds(circles);
}

bool donkeev::contains(const Circle& c, const Point& p)
{
  double dx = p.x - c.center.x;
  double dy = p.y - c.center.y;
  return dx * dx + dy * dy <= c.radius * c.radius;
}

bool donkeev::read_circles(std::vector<Circle>& out)
{
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

donkeev::Bounds donkeev::compute_bounds(const std::vector<Circle>& circles) {
  double min_x = double(circles[0].center.x) - double(circles[0].radius);
  double max_x = double(circles[0].center.x) + double(circles[0].radius);
  double min_y = double(circles[0].center.y) - double(circles[0].radius);
  double max_y = double(circles[0].center.y) + double(circles[0].radius);
  
  for (const auto& c : circles) {
    double cx = double(c.center.x);
    double cy = double(c.center.y);
    double r  = double(c.radius);
    min_x = std::min(min_x, cx - r);
    max_x = std::max(max_x, cx + r);
    min_y = std::min(min_y, cy - r);
    max_y = std::max(max_y, cy + r);
  }
  return {min_x, max_x, min_y, max_y};
}
