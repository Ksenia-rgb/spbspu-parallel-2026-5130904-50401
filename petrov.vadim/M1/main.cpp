#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <thread>
#include <vector>

namespace petrov
{
  struct p_t {
    int x_;
    int y_;
  };

  struct Ellipse {
    Ellipse(int r1, int r2, int x, int y):
      center_{x, y},
      radius1_(r1),
      radius2_(r2 > 0 ? r2 : r1)
    {}

    int getR1() const {
      return radius1_;
    }

    int getR2() const {
      return radius2_;
    }

    p_t getCenter() const {
      return center_;
    }

    private:
      p_t center_;
      int radius1_;
      int radius2_;
  };

  struct UniBox {
    int maxX, maxY, minX, minY;
  };

  UniBox calculateBoundingBox(const std::vector< Ellipse >& shapes) {
    UniBox res {
      shapes[0].getCenter().x_ - shapes[0].getR1(),
      shapes[0].getCenter().x_ + shapes[0].getR1(),
      shapes[0].getCenter().y_ - shapes[0].getR2(),
      shapes[0].getCenter().y_ + shapes[0].getR2()};

    for (size_t i = 1; i < shapes.size(); ++i) {
      res.maxX = std::max(res.maxX, shapes[i].getCenter().x_ + shapes[i].getR1());
      res.minX = std::min(res.minX, shapes[i].getCenter().x_ - shapes[i].getR1());
      res.minY = std::min(res.minY, shapes[i].getCenter().y_ - shapes[i].getR2());
      res.maxY = std::max(res.maxY, shapes[i].getCenter().y_ + shapes[i].getR2());
    }

    return res;
  }

  bool isInside(const Ellipse& shape, const p_t& point) {
    const double dx = (point.x_ - shape.getCenter().x_) / shape.getR1();
    const double dy = (point.y_ - shape.getCenter().y_) / shape.getR2();

    return ((dx * dx) + (dy * dy)) <= 1.0;
  }
}

int main(int argc, char const *argv[]) {
  if (argc < 3 || argc > 4) {
    std::cerr << "invalid arguments count\n";
    return 1;
  }

  int threads, tries, seed = 0;
  try {
    threads = std::stoll(argv[1]);
    tries = std::stoll(argv[2]);

    if (argc == 4) {
      seed = std::stoll(argv[3]);
    } else {
      seed = 0;
    }

    if (threads < 1 || tries < 1 || seed < 0) {
      std::cerr << "Arguments invalid\n";
      return 1;
    }
  } catch(...) {
    std::cerr << "Invalid arguments" << '\n';
    return 1;
  }

  std::vector< petrov::Ellipse > shapes;

  int r1, r2, x, y = 0;
  while (std::cin >> r1 >> r2 >> x >> y) {
    if (r1 <= 0 || r2 < 0) {
      std::cerr << "Radius invalid\n";
      return 1;
    }

    shapes.push_back(petrov::Ellipse(r1, r2, x, y));
  }
  
  if (!std::cin.eof()) {
    std::cerr << "Failed to parse figure\n";
    return 1;
  }

  if (shapes.empty()) {
    std::cerr << "No figures provided\n";
    return 1;
  }

  
  return 0;
}
