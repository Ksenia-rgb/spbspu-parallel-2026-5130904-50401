#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <exception>

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

  struct Result {
    long long in_union = 0;
    long long in_intersection = 0;
  };

  bool contains(const Circle&, const Point&);
  bool read_circles(std::vector<Circle>&);
  Bounds compute_bounds(const std::vector<Circle>& circles);
  Result worker(int id, long long my_tries, const std::vector<Circle>& circles,
      const Bounds& b, unsigned seed);
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

  std::vector<std::thread> pool;
  std::vector<donkeev::Result> results(threads);

  long long base = tries / threads;
  long long rem  = tries % threads;

  for (long long i = 0; i < threads; ++i) {
    long long my_tries = base + (i < rem ? 1 : 0);
    pool.emplace_back([&, i, my_tries] {
      results[i] = worker(int(i), my_tries, circles, bounds,
      unsigned(seed));
    });
  }

  for (auto& t : pool) t.join();
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

donkeev::Result donkeev::worker(int id, long long my_tries, const std::vector<Circle>& circles,
    const Bounds& b, unsigned seed)
{
  std::default_random_engine gen(seed);
  
  std::uniform_real_distribution<double> dist_x(b.min_x, b.max_x);
  std::uniform_real_distribution<double> dist_y(b.min_y, b.max_y);
  
  Result res;
  for (long long i = 0; i < my_tries; ++i) {
    double x = dist_x(gen);
    double y = dist_y(gen);
    
    bool in_any = false;
    bool in_all = true;
    for (const auto& c : circles) {
      bool inside = contains(c, {x, y});
      in_any = in_any || inside;
      in_all = in_all && inside;
    }
    
    if (in_any) res.in_union++;
    if (in_all) res.in_intersection++;
  }
  return res;
}
