#include <algorithm>
#include <random>
#include <future>

#include "shape.hpp"

sedov::Box sedov::boundingBox(const Shape & shape)
{
  return Box{shape.x - shape.r, shape.x + shape.r, shape.y - shape.r, shape.y + shape.r};
}

sedov::Box sedov::boundingBox(const std::vector< Shape > & shapes)
{
  Box box = boundingBox(shapes.front());
  for (std::size_t i = 1; i < shapes.size(); ++i) {
    const Box b = boundingBox(shapes[i]);
    box.x_min = std::min(box.x_min, b.x_min);
    box.x_max = std::max(box.x_max, b.x_max);
    box.y_min = std::min(box.y_min, b.y_min);
    box.y_max = std::max(box.y_max, b.y_max);
  }
  return box;
}

bool sedov::isInside(double dx, double dy, double r)
{
  return dx * dx + dy * dy <= r * r;
}

sedov::Result sedov::calc(const std::vector< Shape > & shapes, const Box & box, size_t tests, size_t seed)
{
  std::default_random_engine engine(seed);
  std::uniform_real_distribution< double > dist_x(static_cast< double >(box.x_min), static_cast< double >(box.x_max));
  std::uniform_real_distribution< double > dist_y(static_cast< double >(box.y_min), static_cast< double >(box.y_max));
  size_t hits_union = 0;
  size_t hits_inter = 0;
  for (size_t i = 0; i < tests; ++i) {
    const double x = dist_x(engine);
    const double y = dist_y(engine);
    bool in_any = false;
    bool in_all = true;
    for (size_t j = 0; j < shapes.size(); ++j) {
      const bool res = isInside(x - static_cast< double >(shapes[j].x), y - static_cast< double >(shapes[j].y),
          static_cast< double >(shapes[j].r));
      in_any = in_any || res;
      in_all = in_all && res;
    }
    hits_union += in_any;
    hits_inter += in_all;
  }
  return Result{hits_union, hits_inter};
}

sedov::Result sedov::countTotalHits(const std::vector< Shape > & shapes, const Box & box, size_t threads, size_t tries, size_t seed)
{
  const size_t base = tries / threads;
  const size_t rem = tries % threads;
  std::vector< std::future< Result > > futures;
  futures.reserve(threads);
  for (size_t i = 0; i < threads; ++i) {
    size_t local_tries = base + (i < rem ? 1 : 0);
    size_t local_seed = seed + i;
    futures.emplace_back(
        std::async(std::launch::async, calc, std::cref(shapes), std::cref(box), local_tries, local_seed));
  }
  Result res{0, 0};
  for (size_t i = 0; i < threads; ++i) {
    const Result local_res = futures[i].get();
    res.hits_union += local_res.hits_union;
    res.hits_inter += local_res.hits_inter;
  }
  return res;
}
