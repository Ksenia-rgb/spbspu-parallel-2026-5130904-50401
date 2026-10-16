#include "area.hpp"

#include <algorithm>
#include <functional>
#include <future>
#include <limits>
#include <random>
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
    std::size_t cover;
    std::size_t comm;
  };

  bool isInside(const petrov::circle_t &circle, double x, double y)
  {
    const double dx = x - circle.x;
    const double dy = y - circle.y;
    return (dx * dx + dy * dy) <= (circle.r * circle.r);
  }

  box_t getBoundingBox(const circles_t &circles)
  {
    const double inf = std::numeric_limits< double >::infinity();
    box_t box = {inf, inf, -inf, -inf};
    for (const petrov::circle_t &circle: circles) {
      box.left = std::min(box.left, circle.x - circle.r);
      box.bottom = std::min(box.bottom, circle.y - circle.r);
      box.right = std::max(box.right, circle.x + circle.r);
      box.top = std::max(box.top, circle.y + circle.r);
    }
    return box;
  }

  hits_t castPoints(const circles_t &circles, const box_t &box, std::size_t tries, std::size_t seed)
  {
    std::mt19937_64 generator(seed);
    std::uniform_real_distribution< double > xDist(box.left, box.right);
    std::uniform_real_distribution< double > yDist(box.bottom, box.top);
    hits_t hits = {0, 0};
    for (std::size_t i = 0; i < tries; ++i) {
      const double x = xDist(generator);
      const double y = yDist(generator);
      auto isHit = [x, y](const petrov::circle_t &circle)
      {
        return isInside(circle, x, y);
      };
      const auto inside = static_cast< std::size_t >(std::count_if(circles.cbegin(), circles.cend(), isHit));
      if (inside != 0) {
        ++hits.cover;
      }
      if (inside == circles.size()) {
        ++hits.comm;
      }
    }
    return hits;
  }
}

petrov::areas_t petrov::estimateAreas(const std::vector< circle_t > &circles, std::size_t threads,
  std::size_t tries, std::size_t seed)
{
  if (circles.empty()) {
    return areas_t{0.0, 0.0};
  }

  const std::size_t firstHelper = 1;
  const std::size_t parts = std::max(threads, firstHelper);
  const std::size_t share = tries / parts;
  const box_t box = getBoundingBox(circles);

  std::vector< std::future< hits_t > > pending;
  pending.reserve(parts);
  for (std::size_t i = firstHelper; i < parts; ++i) {
    auto task = std::async(std::launch::async, castPoints,
        std::cref(circles), std::cref(box), share, seed + i);
    pending.push_back(std::move(task));
  }

  hits_t total = castPoints(circles, box, share + (tries % parts), seed);
  for (std::future< hits_t > &result: pending) {
    const hits_t part = result.get();
    total.cover += part.cover;
    total.comm += part.comm;
  }

  const double boxArea = (box.right - box.left) * (box.top - box.bottom);
  const double count = static_cast< double >(tries);
  const double cover = boxArea * static_cast< double >(total.cover) / count;
  const double comm = boxArea * static_cast< double >(total.comm) / count;
  return areas_t{cover, comm};
}
