#include "area.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <future>
#include <limits>
#include <random>
#include <utility>
#include <vector>

namespace {
  using circles_t = std::vector< petrov::circle_t >;

  struct box_t {
    double left;
    double bottom;
    double right;
    double top;
  };

  struct hits_t {
    std::size_t covered;
    std::size_t common;
  };

  std::size_t getPartSize(std::size_t tries, std::size_t parts, std::size_t index)
  {
    std::size_t size = tries / parts;
    if (index < (tries % parts)) {
      ++size;
    }
    return size;
  }

  bool isInside(const petrov::circle_t &circle, double x, double y)
  {
    const double dx = x - circle.x;
    const double dy = y - circle.y;
    return (dx * dx + dy * dy) <= (circle.radius * circle.radius);
  }

  box_t getBoundingBox(const circles_t &circles)
  {
    const double inf = std::numeric_limits< double >::infinity();
    box_t box = {inf, inf, -inf, -inf};
    for (const petrov::circle_t &circle : circles) {
      box.left = std::min(box.left, circle.x - circle.radius);
      box.bottom = std::min(box.bottom, circle.y - circle.radius);
      box.right = std::max(box.right, circle.x + circle.radius);
      box.top = std::max(box.top, circle.y + circle.radius);
    }
    return box;
  }

  hits_t castPoints(const circles_t &circles, const box_t &box, std::size_t tries, std::size_t seed)
  {
    std::default_random_engine generator(seed);
    std::uniform_real_distribution< double > x_dist(box.left, box.right);
    std::uniform_real_distribution< double > y_dist(box.bottom, box.top);
    hits_t hits = {0, 0};
    for (std::size_t i = 0; i < tries; ++i) {
      const double x = x_dist(generator);
      const double y = y_dist(generator);
      auto is_hit = [x, y](const petrov::circle_t &circle)
      {
        return isInside(circle, x, y);
      };
      const auto inside = static_cast< std::size_t >(std::count_if(circles.cbegin(), circles.cend(), is_hit));
      if (inside != 0) {
        ++hits.covered;
      }
      if (inside == circles.size()) {
        ++hits.common;
      }
    }
    return hits;
  }
}

petrov::areas_t petrov::estimateAreas(
    const std::vector< circle_t > &circles, std::size_t threads, std::size_t tries, std::size_t seed)
{
  if (circles.empty()) {
    return areas_t{0.0, 0.0};
  }

  const std::size_t first_helper = 1;
  const std::size_t parts = std::max(threads, first_helper);
  const box_t box = getBoundingBox(circles);

  std::vector< std::future< hits_t > > pending;
  pending.reserve(parts);
  for (std::size_t i = first_helper; i < parts; ++i) {
    const std::size_t part_size = getPartSize(tries, parts, i);
    auto task = std::async(std::launch::async, castPoints, std::cref(circles), std::cref(box), part_size, seed + i);
    pending.push_back(std::move(task));
  }

  hits_t total = castPoints(circles, box, getPartSize(tries, parts, 0), seed);
  for (std::future< hits_t > &result : pending) {
    const hits_t part = result.get();
    total.covered += part.covered;
    total.common += part.common;
  }

  const double box_area = (box.right - box.left) * (box.top - box.bottom);
  const double count = static_cast< double >(tries);
  const double covered = box_area * static_cast< double >(total.covered) / count;
  const double common = box_area * static_cast< double >(total.common) / count;
  return areas_t{covered, common};
}
