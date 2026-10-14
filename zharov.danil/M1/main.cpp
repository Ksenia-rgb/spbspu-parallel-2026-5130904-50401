#include <algorithm>
#include <iostream>
#include <vector>
#include <random>
#include <future>
#include <cstddef>
#include <utility>
#include <string>
#include <exception>

namespace zharov
{
  struct Point
  {
    double x, y;
  };

  struct Circle
  {
    size_t r;
    Point center;
  };

  struct Canvas
  {
    Point left_lower;
    Point right_upper;
  };

  Canvas getCanvas(const std::vector< Circle >& shapes);
  std::pair< size_t, size_t > calcInside(const std::vector< Circle >& shapes, Canvas cv, size_t tests, size_t seed);
  bool isInside(Point pt, const Circle& shape);
  double getArea(size_t inside, size_t tests, Canvas cv);

}

int main(int argc, char** argv)
{
  constexpr int min_args = 3;
  constexpr int max_args = 4;
  constexpr int tries_arg = 2;
  constexpr int seed_arg = 3;
  if (argc < min_args)
  {
    std::cerr << "not enough args\n";
    return 1;
  }
  if (argc > max_args)
  {
    std::cerr << "too many args\n";
    return 1;
  }

  long long threads = 0, tries = 0, seed = 0;
  constexpr long long max_threads = 1000;
  try
  {
    threads = std::min(std::stoll(argv[1]), max_threads);
    tries = std::stoll(argv[tries_arg]);
    if (argc == max_args)
    {
      seed = std::stoll(argv[seed_arg]);
    }
  }
  catch (...)
  {
    std::cerr << "incorrect args\n";
    return 1;
  }
  threads = threads == 0 ? 1 : threads;

  if (threads <= 0 || tries <= 0 || seed < 0)
  {
    std::cerr << "args must be positive\n";
    return 1;
  }

  size_t skip = 0;
  zharov::Circle shape{0, 0, 0};
  std::vector< zharov::Circle > shapes;
  while (std::cin >> shape.r)
  {
    if (!(std::cin >> skip >> shape.center.x >> shape.center.y))
    {
      std::cerr << "bad input\n";
      return 1;
    }
    shapes.push_back(shape);
  }
  if (!std::cin.eof())
  {
    std::cerr << "bad input\n";
    return 1;
  }
  if (shapes.empty())
  {
    std::cout << 0 << " " << 0 << "\n";
    return 0;
  }

  zharov::Canvas cv = getCanvas(shapes);
  size_t inside = 0, each_inside = 0;
  try
  {
    std::vector< std::future< std::pair< size_t, size_t > > > results;
    results.reserve(threads);
    const size_t th = static_cast< size_t >(threads);
    size_t nums_on_thread = tries / th;
    {
      for (size_t i = 0; i < th - 1; ++i)
      {
        results.push_back(std::async(std::launch::async, zharov::calcInside, shapes, cv, nums_on_thread, seed++));
      }
      results.push_back(
          std::async(std::launch::async, zharov::calcInside, shapes, cv, nums_on_thread + tries % th, seed));

      for (size_t i = 0; i < th; ++i)
      {
        const std::pair< size_t, size_t > res = results[i].get();
        inside += res.first;
        each_inside += res.second;
      }
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << "\n";
    return 1;
  }

  const double area = zharov::getArea(inside, tries, cv);
  const double intersection_area = zharov::getArea(each_inside, tries, cv);

  std::cout << area << " " << intersection_area << "\n";
}

zharov::Canvas zharov::getCanvas(const std::vector< Circle >& shapes)
{
  const Circle& first = shapes.front();
  Canvas res;
  res.left_lower.x = first.center.x - first.r;
  res.left_lower.y = first.center.y - first.r;
  res.right_upper.x = first.center.x + first.r;
  res.right_upper.y = first.center.y + first.r;
  for (auto i = shapes.cbegin() + 1; i != shapes.cend(); ++i)
  {
    res.left_lower.x = std::min(i->center.x - i->r, res.left_lower.x);
    res.left_lower.y = std::min(i->center.y - i->r, res.left_lower.y);
    res.right_upper.x = std::max(i->center.x + i->r, res.right_upper.x);
    res.right_upper.y = std::max(i->center.y + i->r, res.right_upper.y);
  }
  return res;
}

std::pair< size_t, size_t > zharov::calcInside(
    const std::vector< Circle >& shapes, Canvas cv, size_t tests, size_t seed)
{
  std::default_random_engine eng(seed);
  std::uniform_real_distribution< double > dist_high(cv.left_lower.y, cv.right_upper.y);
  std::uniform_real_distribution< double > dist_width(cv.left_lower.x, cv.right_upper.x);
  size_t inside_each = 0, inside = 0;
  for (size_t i = 0; i < tests; ++i)
  {
    const Point pt{dist_width(eng), dist_high(eng)};
    size_t count = 0;
    for (auto j = shapes.cbegin(); j != shapes.cend(); ++j)
    {
      count += isInside(pt, *j);
    }
    if (count > 0)
    {
      ++inside;
      if (count == shapes.size())
      {
        ++inside_each;
      }
    }
  }

  return {inside, inside_each};
}

bool zharov::isInside(Point pt, const Circle& shape)
{
  const double dx = pt.x - shape.center.x;
  const double dy = pt.y - shape.center.y;
  const double r = shape.r;
  return dx * dx + dy * dy <= r * r;
}

double zharov::getArea(size_t inside, size_t tests, Canvas cv)
{
  return ((cv.right_upper.x - cv.left_lower.x) * (cv.right_upper.y - cv.left_lower.y)) * inside / tests;
}
