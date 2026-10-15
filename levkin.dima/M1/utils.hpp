#ifndef UTILS_HPP
#define UTILS_HPP

#include <array>
#include <istream>
#include <vector>

namespace levkin {
  using BorderRange = std::array< int, 2 >;

  struct Circle {
    int x, y, radius;
  };

  struct AnyAllResult {
    size_t any, all = 0;

    AnyAllResult &operator+=(const AnyAllResult &other)
    {
      any += other.any;
      all += other.all;
      return *this;
    }
  };

  struct CirclesWithRanges {
    struct Ranges {
      BorderRange xRange;
      BorderRange yRange;
    };

    std::vector< Circle > circles{};
    Ranges ranges;

    void updateRanges();
    size_t rangeArea() const;
  };
  std::istream &operator>>(std::istream &is, Circle &c);

  CirclesWithRanges getData(std::istream &is);
  bool gotIn(levkin::Circle c, double x, double y);
  AnyAllResult gotIn(CirclesWithRanges const &cv, double x, double y);
  AnyAllResult runTests(size_t howManyTests, size_t seed, CirclesWithRanges const &cv);
  std::array< BorderRange, 2 > getBorderSquare(Circle const &c);

}
#endif
