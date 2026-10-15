#include "shapes.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace burukov
{

  Circle::Circle(double radius, double x, double y) noexcept:
    radius_(radius),
    x_(x),
    y_(y)
  {}

  bool Circle::contains(double x, double y) const noexcept
  {
    const double dx = x - x_;
    const double dy = y - y_;
    return (dx * dx + dy * dy) <= (radius_ * radius_);
  }

  double Circle::getMinX() const noexcept
  {
    return x_ - radius_;
  }

  double Circle::getMinY() const noexcept
  {
    return y_ - radius_;
  }

  double Circle::getMaxX() const noexcept
  {
    return x_ + radius_;
  }

  double Circle::getMaxY() const noexcept
  {
    return y_ + radius_;
  }

  Ellipse::Ellipse(double a, double b, double x, double y) noexcept:
    a_(a),
    b_(b),
    x_(x),
    y_(y)
  {}

  bool Ellipse::contains(double x, double y) const noexcept
  {
    const double dx = (x - x_) / a_;
    const double dy = (y - y_) / b_;
    return (dx * dx + dy * dy) <= 1.0;
  }

  double Ellipse::getMinX() const noexcept
  {
    return x_ - a_;
  }

  double Ellipse::getMinY() const noexcept
  {
    return y_ - b_;
  }

  double Ellipse::getMaxX() const noexcept
  {
    return x_ + a_;
  }

  double Ellipse::getMaxY() const noexcept
  {
    return y_ + b_;
  }

  rectangle_t findRectangleRange(const std::vector< std::unique_ptr< Shape > >& shapes)
  {
    rectangle_t rect = {shapes[0]->getMinX(), shapes[0]->getMinY(), shapes[0]->getMaxX(), shapes[0]->getMaxY()};

    for (std::size_t i = 1; i < shapes.size(); ++i)
    {
      rect.min_x = std::min(rect.min_x, shapes[i]->getMinX());
      rect.min_y = std::min(rect.min_y, shapes[i]->getMinY());
      rect.max_x = std::max(rect.max_x, shapes[i]->getMaxX());
      rect.max_y = std::max(rect.max_y, shapes[i]->getMaxY());
    }
    return rect;
  }

  long long parseArgument(const char* arg, const std::string& param)
  {
    std::size_t pos = 0;
    const long long value = std::stoll(arg, &pos);
    if (pos != std::string(arg).length())
    {
      throw std::invalid_argument("Invalid characters in " + param);
    }
    return value;
  }

  std::pair< long long, long long > calc(long long tries, long long seed, double min_x, double max_x, double min_y,
      double max_y, const std::vector< std::unique_ptr< Shape > >& shapes)
  {
    long long count_in_any = 0;
    long long count_in_all = 0;

    std::default_random_engine engine(static_cast< unsigned int >(seed));
    std::uniform_real_distribution< double > dist_x(min_x, max_x);
    std::uniform_real_distribution< double > dist_y(min_y, max_y);

    for (long long i = 0; i < tries; ++i)
    {
      const double x = dist_x(engine);
      const double y = dist_y(engine);

      bool in_any = false;
      bool in_all = true;

      for (std::size_t j = 0; j < shapes.size(); ++j)
      {
        if (shapes[j]->contains(x, y))
        {
          in_any = true;
        }
        else
        {
          in_all = false;
        }
      }

      if (in_any)
      {
        ++count_in_any;
      }
      if (in_all)
      {
        ++count_in_all;
      }
    }
    return std::make_pair(count_in_any, count_in_all);
  }

  std::pair< double, double > area(long long threads, long long tries, long long seed,
      const std::vector< std::unique_ptr< Shape > >& shapes, rectangle_t rect)
  {
    const long long chunk = tries / threads;
    const long long remainder = tries % threads;

    std::vector< std::pair< long long, long long > > results(static_cast< std::size_t >(threads));
    std::vector< std::thread > thread_pool;
    thread_pool.reserve(static_cast< std::size_t >(threads));

    for (long long j = 0; j < threads; ++j)
    {
      const long long local_seed = seed + j;
      const long long local_tries = (j == threads - 1) ? (chunk + remainder) : chunk;

      thread_pool.emplace_back(
          [&, j, local_tries, local_seed]()
          {
            results[static_cast< std::size_t >(j)]
                = calc(local_tries, local_seed, rect.min_x, rect.max_x, rect.min_y, rect.max_y, shapes);
          });
    }

    for (std::size_t i = 0; i < thread_pool.size(); ++i)
    {
      thread_pool[i].join();
    }

    long long total_union_hits = 0;
    long long total_intersect_hits = 0;

    for (std::size_t i = 0; i < results.size(); ++i)
    {
      total_union_hits += results[i].first;
      total_intersect_hits += results[i].second;
    }

    const double rect_area = (rect.max_x - rect.min_x) * (rect.max_y - rect.min_y);
    const double area_union = static_cast< double >(total_union_hits) / static_cast< double >(tries) * rect_area;
    const double area_intersect
        = static_cast< double >(total_intersect_hits) / static_cast< double >(tries) * rect_area;

    return std::make_pair(area_union, area_intersect);
  }

}
