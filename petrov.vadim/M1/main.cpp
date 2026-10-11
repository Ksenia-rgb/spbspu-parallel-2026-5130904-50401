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
    UniBox res{
      shapes[0].getCenter().x_ + shapes[0].getR1(),
      shapes[0].getCenter().y_ + shapes[0].getR2(),
      shapes[0].getCenter().x_ - shapes[0].getR1(),
      shapes[0].getCenter().y_ - shapes[0].getR2()};

    for (size_t i = 1; i < shapes.size(); ++i) {
      res.maxX = std::max(res.maxX, shapes[i].getCenter().x_ + shapes[i].getR1());
      res.minX = std::min(res.minX, shapes[i].getCenter().x_ - shapes[i].getR1());
      res.minY = std::min(res.minY, shapes[i].getCenter().y_ - shapes[i].getR2());
      res.maxY = std::max(res.maxY, shapes[i].getCenter().y_ + shapes[i].getR2());
    }

    return res;
  }

  bool isInside(const Ellipse& shape, const p_t& point) {
    const double dx = (point.x_ - shape.getCenter().x_) / static_cast< double >(shape.getR1());
    const double dy = (point.y_ - shape.getCenter().y_) / static_cast< double >(shape.getR2());

    return ((dx * dx) + (dy * dy)) <= 1.0;
  }

  void calculateAreas(int localTries, size_t localSeed, UniBox box,
    const std::vector< Ellipse >& shapes, int* outUnionHits, int* outIntersectionHits)
  {
    std::mt19937 rng(localSeed);
    std::uniform_int_distribution< int > distX(box.minX, box.maxX);
    std::uniform_int_distribution< int > distY(box.minY, box.maxY);

    int unionHits = 0;
    int intersectionHits = 0;
    const size_t shapesCount = shapes.size();

    for (int i = 0; i < localTries; ++i) {
      const p_t pt{distX(rng), distY(rng)};
      size_t hitCount = 0;

      for (size_t j = 0; j < shapesCount; ++j) {
        if (isInside((shapes)[j], pt)) {
          ++hitCount;
        }
      }

      if (hitCount > 0) {
        ++unionHits;
      }
      if (hitCount == shapesCount) {
        ++intersectionHits;
      }
    }

    *outUnionHits = unionHits;
    *outIntersectionHits = intersectionHits;
  }
}

int main(int argc, char const *argv[]) {
  if (argc < 3 || argc > 4) {
    std::cerr << "invalid arguments count\n";
    return 1;
  }

  int threads = 0;
  int tries = 0;
  int seed = 0;
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

  int r1 = 0;
  int r2 = 0;
  int x = 0;
  int y = 0;
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

  const petrov::UniBox box = petrov::calculateBoundingBox(shapes);
  const double boxArea = static_cast< double >(box.maxX - box.minX)
    * static_cast< double >(box.maxY - box.minY);

  const int actualThreads = std::min(threads, tries);
  const int baseTries = tries / actualThreads;
  const int remainderTries = tries % actualThreads;

  std::vector< std::thread > workers;
  workers.reserve(actualThreads);

  std::vector< int > threadUnionHits(actualThreads, 0);
  std::vector< int > threadIntersectionHits(actualThreads, 0);

  for (int i = 0; i < actualThreads; ++i) {
    const int currentTries = baseTries + ((i < remainderTries) ? 1 : 0);
    const size_t currentSeed = static_cast< size_t >(seed + i);

    workers.emplace_back(
      petrov::calculateAreas,
      currentTries,
      currentSeed,
      box,
      std::cref(shapes),
      &threadUnionHits[i],
      &threadIntersectionHits[i]);
  }

  int totalUnionHits = 0;
  int totalIntersectionHits = 0;

  for (int i = 0; i < actualThreads; ++i) {
    workers[i].join();
    totalUnionHits += threadUnionHits[i];
    totalIntersectionHits += threadIntersectionHits[i];
  }

  const double unionArea = boxArea * (static_cast< double >(totalUnionHits) / tries);
  const double intersectionArea = boxArea * (static_cast< double >(totalIntersectionHits) / tries);

  std::cout.precision(std::numeric_limits< double >::max_digits10);
  std::cout << unionArea << ' ' << intersectionArea << '\n';

  return 0;
}
