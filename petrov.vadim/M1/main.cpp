#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <thread>
#include <vector>

namespace petrov {
  struct p_t {
    int x;
    int y;
  };

  struct Ellipse {
    Ellipse(int r1, int r2, int x, int y):
      center_{x, y},
      radius1_(r1),
      radius2_(r2 > 0 ? r2 : r1)
    {}

    int getR1() const
    {
      return radius1_;
    }

    int getR2() const
    {
      return radius2_;
    }

    p_t getCenter() const
    {
      return center_;
    }

  private:
    p_t center_;
    int radius1_;
    int radius2_;
  };

  struct UniBox {
    int max_x;
    int max_y;
    int min_x;
    int min_y;
  };

  UniBox calculateBoundingBox(const std::vector< Ellipse >& shapes)
  {
    UniBox res{
      shapes[0].getCenter().x + shapes[0].getR1(),
      shapes[0].getCenter().y + shapes[0].getR2(),
      shapes[0].getCenter().x - shapes[0].getR1(),
      shapes[0].getCenter().y - shapes[0].getR2()};

    for (size_t i = 1; i < shapes.size(); ++i) {
      res.max_x = std::max(res.max_x, shapes[i].getCenter().x + shapes[i].getR1());
      res.min_x = std::min(res.min_x, shapes[i].getCenter().x - shapes[i].getR1());
      res.min_y = std::min(res.min_y, shapes[i].getCenter().y - shapes[i].getR2());
      res.max_y = std::max(res.max_y, shapes[i].getCenter().y + shapes[i].getR2());
    }

    return res;
  }

  void calculateAreas(
    int local_tries,
    size_t local_seed,
    UniBox box,
    const std::vector< Ellipse >& shapes,
    int* out_union_hits,
    int* out_intersection_hits)
  {
    std::mt19937_64 rng(local_seed);
    std::uniform_real_distribution< double > dist_x(
      static_cast< double >(box.min_x),
      static_cast< double >(box.max_x));
    std::uniform_real_distribution< double > dist_y(
      static_cast< double >(box.min_y),
      static_cast< double >(box.max_y));

    int union_hits = 0;
    int intersection_hits = 0;
    const size_t shapes_count = shapes.size();

    for (int i = 0; i < local_tries; ++i) {
      const double px = dist_x(rng);
      const double py = dist_y(rng);
      size_t hit_count = 0;

      for (size_t j = 0; j < shapes_count; ++j) {
        const double dx = (px - static_cast< double >(shapes[j].getCenter().x))
          / static_cast< double >(shapes[j].getR1());
        const double dy = (py - static_cast< double >(shapes[j].getCenter().y))
          / static_cast< double >(shapes[j].getR2());

        if (((dx * dx) + (dy * dy)) <= 1.0) {
          ++hit_count;
        }
      }

      if (hit_count > 0) {
        ++union_hits;
      }
      if (hit_count == shapes_count) {
        ++intersection_hits;
      }
    }

    *out_union_hits = union_hits;
    *out_intersection_hits = intersection_hits;
  }
}

int main(int argc, char const* argv[])
{
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

    if (threads < 0 || tries <= 0 || seed < 0) {
      std::cerr << "Arguments invalid\n";
      return 1;
    }
  } catch (...) {
    std::cerr << "Invalid arguments\n";
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
  const double box_area = static_cast< double >(box.max_x - box.min_x)
    * static_cast< double >(box.max_y - box.min_y);

  const int actual_threads = (threads == 0) ? 1 : std::min(threads, tries);
  const int base_tries = tries / actual_threads;
  const int remainder_tries = tries % actual_threads;

  std::vector< std::thread > workers;
  workers.reserve(actual_threads);

  std::vector< int > threadunion_hits(actual_threads, 0);
  std::vector< int > threadintersection_hits(actual_threads, 0);

  for (int i = 0; i < actual_threads; ++i) {
    const int current_tries = base_tries + ((i < remainder_tries) ? 1 : 0);
    const size_t current_seed = static_cast< size_t >(seed + i);

    workers.emplace_back(
      petrov::calculateAreas,
      current_tries,
      current_seed,
      box,
      std::cref(shapes),
      &threadunion_hits[i],
      &threadintersection_hits[i]);
  }

  int totalunion_hits = 0;
  int totalintersection_hits = 0;

  for (int i = 0; i < actual_threads; ++i) {
    workers[i].join();
    totalunion_hits += threadunion_hits[i];
    totalintersection_hits += threadintersection_hits[i];
  }

  const double union_area = box_area * (static_cast< double >(totalunion_hits) / tries);
  const double intersection_area = box_area * (static_cast< double >(totalintersection_hits) / tries);

  std::cout.precision(std::numeric_limits< double >::max_digits10);
  std::cout << union_area << ' ' << intersection_area << '\n';

  return 0;
}
