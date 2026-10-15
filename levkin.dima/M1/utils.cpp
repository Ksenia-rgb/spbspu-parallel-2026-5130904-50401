#include "utils.hpp"

#include <array>
#include <iostream>
#include <istream>
#include <random>
#include <vector>

namespace levkin {
  std::istream &operator>>(std::istream &is, Circle &c)
  {
    int dummy = 0;
    is >> c.radius >> dummy >> c.x >> c.y;

    return is;
  }

  void CirclesWithRanges::updateRanges()
  {
    if (circles.empty()) {
      ranges.xRange = {0, 0};
      ranges.yRange = {0, 0};
      return;
    }

    int minX = std::numeric_limits< int >::max();
    int maxX = std::numeric_limits< int >::min();
    int minY = std::numeric_limits< int >::max();
    int maxY = std::numeric_limits< int >::min();

    for (const Circle &c : circles) {
      std::array< BorderRange, 2 > borderSquare = getBorderSquare(c);
      minX = std::min(minX, borderSquare[0][0]);
      maxX = std::max(maxX, borderSquare[0][1]);
      minY = std::min(minY, borderSquare[1][0]);
      maxY = std::max(maxY, borderSquare[1][1]);
    }

    ranges.xRange = {minX, maxX};
    ranges.yRange = {minY, maxY};
  }
  size_t CirclesWithRanges::rangeArea() const
  {
    return (ranges.xRange[1] - ranges.xRange[0]) * (ranges.yRange[1] - ranges.yRange[0]);
  }

  std::array< BorderRange, 2 > getBorderSquare(Circle const &c)
  {
    std::array< BorderRange, 2 > result;
    result[0][0] = c.x - c.radius;
    result[0][1] = c.x + c.radius;
    result[1][0] = c.y - c.radius;
    result[1][1] = c.y + c.radius;

    return result;
  }
  bool gotIn(Circle c, double x, double y)
  {
    double dx = c.x - x;
    double dy = c.y - y;

    return dx * dx + dy * dy <= c.radius * static_cast< double >(c.radius);
  }

  AnyAllResult gotIn(CirclesWithRanges const &cv, double x, double y)
  {
    bool hitAny = false;
    bool hitAll = true;
    for (Circle const &c : cv.circles) {
      bool hit = gotIn(c, x, y);
      hitAny = hitAny || hit;
      hitAll = hitAll && hit;
    }

    AnyAllResult result;
    result.any = hitAny ? 1 : 0;
    result.all = hitAll ? 1 : 0;
    return result;
  }

  CirclesWithRanges getData(std::istream &is)
  {
    Circle dummyCircle{};
    CirclesWithRanges dataVec{};

    while (is >> dummyCircle) {
      dataVec.circles.emplace_back(dummyCircle);
    }

    if (!is.eof()) {
      throw std::invalid_argument("bad input file");
    }

    dataVec.updateRanges();
    return dataVec;
  }
  AnyAllResult runTests(size_t howManyTests, size_t seed, CirclesWithRanges const &cv)
  {
    std::mt19937 gen(seed);
    AnyAllResult result{0, 0};

    BorderRange xRng = cv.ranges.xRange;
    BorderRange yRng = cv.ranges.yRange;

    std::uniform_real_distribution< double > xDist(xRng[0], xRng[1]);
    std::uniform_real_distribution< double > yDist(yRng[0], yRng[1]);

    for (size_t testCount = 0; testCount < howManyTests; ++testCount) {
      double randX = xDist(gen);
      double randY = yDist(gen);

      result += gotIn(cv, randX, randY);
    }

    return result;
  }
} // namespace levkin
