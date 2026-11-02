#include "monte_carlo.hpp"
#include <future>
#include <random>

size_t zubarev::calc(const std::vector< Circle >& circles, size_t tests, size_t seed, const Frame& f)
{
  if (tests == 0 || circles.empty()) {
    return 0;
  }

  std::mt19937 gen(seed);
  std::uniform_real_distribution< double > dist_x(f.min_x, f.max_x);
  std::uniform_real_distribution< double > dist_y(f.min_y, f.max_y);

  size_t hits = 0;
  for (size_t j = 0; j < tests; ++j) {
    const double x = dist_x(gen);
    const double y = dist_y(gen);
    for (const auto& c : circles) {
      if (isInside(c, x, y)) {
        hits++;
        break;
      }
    }
  }
  return hits;
}

double zubarev::area(const std::vector< Circle >& circles, size_t threads, size_t tests, size_t seed)
{
  if (circles.empty() || tests == 0) {
    return 0.0;
  }

  if (threads == 0) {
    threads = 1;
  }

  Frame f = frame(circles);
  const double frame_area = (f.max_x - f.min_x) * (f.max_y - f.min_y);

  std::vector< std::future< size_t > > tasks(threads);

  for (size_t i = 0; i < threads; ++i) {
    size_t tests_per_thread = tests / threads;
    if (i < tests % threads) {
      tests_per_thread += 1;
    }

    tasks[i] = std::async(std::launch::async, calc, std::cref(circles), tests_per_thread, seed + i, f);
  }

  size_t total_hits = 0;
  for (size_t i = 0; i < threads; ++i) {
    total_hits += tasks[i].get();
  }

  return frame_area * (static_cast< double >(total_hits) / tests);
}
