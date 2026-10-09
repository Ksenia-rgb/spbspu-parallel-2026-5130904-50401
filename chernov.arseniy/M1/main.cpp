#include <algorithm>
#include <functional>
#include <future>
#include <iostream>
#include <random>
#include <vector>

namespace chernov {
  struct point_t {
    double x, y;
  };

  struct circle_t {
    point_t center;
    double radius;
  };

  struct frame_t {
    point_t left_bottom;
    point_t right_top;
  };

  void inputCircles(std::vector< circle_t > &circles)
  {
    double radius = 0, _ = 0, x = 0, y = 0;
    while (std::cin >> radius >> _ >> x >> y) {
      circles.push_back(circle_t{.center = {.x = x, .y = y}, .radius = radius});
    }

    if (!std::cin.eof()) {
      throw std::runtime_error("input error");
    }
  }

  bool isInside(const double x, const double y, const circle_t &circle)
  {
    const double dx = x - circle.center.x;
    const double dy = y - circle.center.y;
    const double r = circle.radius;
    return dx * dx + dy * dy <= r * r;
  }

  size_t circlesOf(point_t point, const std::vector< circle_t > &circles)
  {
    const auto bound_is_inside = std::bind(isInside, point.x, point.y, std::placeholders::_1);
    return std::count_if(circles.begin(), circles.end(), bound_is_inside);
  }

  std::pair< size_t, size_t > calc(
      const std::vector< circle_t > &circles, const frame_t &frame, const size_t tests, const size_t seed)
  {
    std::mt19937 gen(seed);
    std::uniform_real_distribution<> dis_x(frame.left_bottom.x, frame.right_top.x);
    std::uniform_real_distribution<> dis_y(frame.left_bottom.y, frame.right_top.y);

    size_t count_in_any = 0;
    size_t count_in_all = 0;
    for (size_t i = 0; i < tests; ++i) {
      const double x = dis_x(gen);
      const double y = dis_y(gen);
      const size_t count_insides = circlesOf({.x = x, .y = y}, circles);

      if (count_insides == circles.size()) {
        ++count_in_all;
      }
      if (count_insides) {
        ++count_in_any;
      }
    }
    return {count_in_any, count_in_all};
  }

  frame_t getFrameOf(const std::vector< circle_t > &circles)
  {
    auto cmp_min_x = [](const circle_t &a, const circle_t &b)
    {
      return a.center.x - a.radius < b.center.x - b.radius;
    };
    auto cmp_min_y = [](const circle_t &a, const circle_t &b)
    {
      return a.center.y - a.radius < b.center.y - b.radius;
    };
    auto cmp_max_x = [](const circle_t &a, const circle_t &b)
    {
      return a.center.x + a.radius < b.center.x + b.radius;
    };
    auto cmp_max_y = [](const circle_t &a, const circle_t &b)
    {
      return a.center.y + a.radius < b.center.y + b.radius;
    };
    const auto it_min_x = std::min_element(circles.begin(), circles.end(), cmp_min_x);
    const auto it_min_y = std::min_element(circles.begin(), circles.end(), cmp_min_y);
    const auto it_max_x = std::max_element(circles.begin(), circles.end(), cmp_max_x);
    const auto it_max_y = std::max_element(circles.begin(), circles.end(), cmp_max_y);
    const double min_x = it_min_x->center.x - it_min_x->radius;
    const double min_y = it_min_y->center.y - it_min_y->radius;
    const double max_x = it_max_x->center.x + it_max_x->radius;
    const double max_y = it_max_y->center.y + it_max_y->radius;
    const frame_t frame{.left_bottom = {.x = min_x, .y = min_y}, .right_top = {.x = max_x, .y = max_y}};
    return frame;
  }

  std::pair< double, double > areaOf(const std::vector< circle_t > &circles, size_t threads, size_t tries, size_t seed)
  {
    frame_t frame = getFrameOf(circles);

    std::vector< std::future< std::pair< size_t, size_t > > > futures;
    const size_t batch = tries / threads;
    futures.emplace_back(std::async(std::launch::async, calc, std::cref(circles), frame, batch + tries % threads, seed));
    for (size_t i = 1; i < threads; ++i) {
      futures.emplace_back(std::async(std::launch::async, calc, std::cref(circles), frame, batch, seed + i));
    }

    size_t count_in_any = 0;
    size_t count_in_all = 0;
    for (size_t i = 0; i < threads; ++i) {
      const std::pair< size_t, size_t > thread_result = futures[i].get();
      count_in_any += thread_result.first;
      count_in_all += thread_result.second;
    }

    const double frame_area = (frame.right_top.x - frame.left_bottom.x) * (frame.right_top.y - frame.left_bottom.y);
    const double union_area = frame_area * count_in_any / tries;
    const double intersection_area = frame_area * count_in_all / tries;
    return {union_area, intersection_area};
  }
}

int main(int argc, char **argv)
{
  using namespace chernov;

  if (argc < 3 || argc > 4) {
    std::cerr << "count of arguments is incorrect\n";
    return 1;
  }

  long long threads = 0, tries = 0, seed = 0;

  try {
    threads = std::stoll(argv[1]);
  } catch (const std::invalid_argument &e) {
    std::cerr << "count of threads is incorrect\n";
    return 1;
  }
  if (threads < 0) {
    std::cerr << "count of threads is incorrect\n";
    return 1;
  }
  if (threads == 0) {
    threads = 1;
  }

  try {
    tries = std::stoll(argv[2]);
  } catch (const std::invalid_argument &e) {
    std::cerr << "count of tries is incorrect\n";
    return 1;
  }
  if (tries < 1) {
    std::cerr << "count of tries is incorrect\n";
    return 1;
  }

  if (argc == 4) {
    try {
      seed = std::stoll(argv[3]);
    } catch (const std::invalid_argument &e) {
      std::cerr << "seed is incorrect\n";
      return 1;
    }
    if (seed < 0) {
      std::cerr << "seed is incorrect\n";
      return 1;
    }
  }

  std::vector< circle_t > circles;
  try {
    inputCircles(circles);
  } catch (const std::runtime_error &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }

  const std::pair< double, double > area = areaOf(circles, threads, tries, seed);
  const double union_area = area.first;
  const double intersection_area = area.second;
  std::cout << union_area << " " << intersection_area << "\n";
}
