#include "monte_carlo.hpp"
#include <future>
#include <random>

namespace
{
  inline double rand_double_53(std::mt19937& gen)
  {
    uint64_t a = gen() >> 5;
    uint64_t b = gen() >> 6;
    return (a * 67108864.0 + b) * (1.0 / 9007199254740992.0);
  }
}

size_t zubarev::calc(const std::vector< Circle >& circles, size_t tests, size_t seed, const Frame& f)
{
  if (tests == 0 || circles.empty()) {
    return 0;
  }

  std::mt19937 gen(seed);
  size_t hits = 0;

  if (tests > 2000000) {
    const double min_x = f.min_x;
    const double min_y = f.min_y;
    const double scale_x = f.max_x - min_x;
    const double scale_y = f.max_y - min_y;

    if (circles.size() == 1) {
      const double cx = circles[0].x;
      const double cy = circles[0].y;
      const double r2 = circles[0].r * circles[0].r;

      for (size_t j = 0; j < tests; ++j) {
        double x = min_x + rand_double_53(gen) * scale_x;
        double y = min_y + rand_double_53(gen) * scale_y;
        double dx = x - cx;
        double dy = y - cy;
        if (dx * dx + dy * dy <= r2) {
          hits++;
        }
      }
      return hits;
    }

    const size_t n = circles.size();
    std::vector< double > r2(n);
    for (size_t i = 0; i < n; ++i) {
      r2[i] = circles[i].r * circles[i].r;
    }

    for (size_t j = 0; j < tests; ++j) {
      double x = min_x + rand_double_53(gen) * scale_x;
      double y = min_y + rand_double_53(gen) * scale_y;
      for (size_t i = 0; i < n; ++i) {
        double dx = x - circles[i].x;
        double dy = y - circles[i].y;
        if (dx * dx + dy * dy <= r2[i]) {
          hits++;
          break;
        }
      }
    }
    return hits;
  }

  // Стандартное распределение для умеренных объёмов испытаний
  std::uniform_real_distribution< double > dist_x(f.min_x, f.max_x);
  std::uniform_real_distribution< double > dist_y(f.min_y, f.max_y);

  if (circles.size() == 1) {
    const double cx = circles[0].x;
    const double cy = circles[0].y;
    const double r2 = circles[0].r * circles[0].r;

    for (size_t j = 0; j < tests; ++j) {
      double x = dist_x(gen);
      double y = dist_y(gen);
      double dx = x - cx;
      double dy = y - cy;
      if (dx * dx + dy * dy <= r2) {
        hits++;
      }
    }
    return hits;
  }

  const size_t n = circles.size();
  std::vector< double > r2(n);
  for (size_t i = 0; i < n; ++i) {
    r2[i] = circles[i].r * circles[i].r;
  }

  for (size_t j = 0; j < tests; ++j) {
    double x = dist_x(gen);
    double y = dist_y(gen);
    for (size_t i = 0; i < n; ++i) {
      double dx = x - circles[i].x;
      double dy = y - circles[i].y;
      if (dx * dx + dy * dy <= r2[i]) {
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

  if (threads == 1) {
    size_t hits = calc(circles, tests, seed, f);
    return frame_area * (static_cast< double >(hits) / tests);
  }

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
