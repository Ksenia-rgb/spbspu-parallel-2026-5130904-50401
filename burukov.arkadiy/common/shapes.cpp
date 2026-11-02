#include "shapes.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace
{
  bool containsData(const burukov::shape_data_t & shape, double x, double y) noexcept
  {
    const double dx = x - shape.x;
    const double dy = y - shape.y;
    if (shape.kind == 0)
    {
      return (dx * dx + dy * dy) <= (shape.a * shape.a);
    }
    const double cos_a = std::cos(shape.angle);
    const double sin_a = std::sin(shape.angle);
    const double local_x = (dx * cos_a + dy * sin_a) / shape.a;
    const double local_y = (-dx * sin_a + dy * cos_a) / shape.b;
    return (local_x * local_x + local_y * local_y) <= 1.0;
  }

  burukov::rectangle_t getSingleDataBound(const burukov::shape_data_t & shape) noexcept
  {
    if (shape.kind == 0)
    {
      return {shape.x - shape.a, shape.y - shape.a, shape.x + shape.a, shape.y + shape.a};
    }
    const double cos_a = std::cos(shape.angle);
    const double sin_a = std::sin(shape.angle);
    const double half_w = std::sqrt((shape.a * cos_a) * (shape.a * cos_a) + (shape.b * sin_a) * (shape.b * sin_a));
    const double half_h = std::sqrt((shape.a * sin_a) * (shape.a * sin_a) + (shape.b * cos_a) * (shape.b * cos_a));
    return {shape.x - half_w, shape.y - half_h, shape.x + half_w, shape.y + half_h};
  }

  long long calcUnionHits(
      long long tries, long long seed, burukov::rectangle_t rect, const std::vector< burukov::shape_data_t > & shapes)
  {
    std::default_random_engine generator(static_cast< unsigned int >(seed));
    const std::default_random_engine::result_type minimum = generator.min();
    const double range = static_cast< double >(generator.max()) - static_cast< double >(minimum) + 1.0;
    const double x_scale = (rect.max_x - rect.min_x) / range;
    const double y_scale = (rect.max_y - rect.min_y) / range;

    long long count_in_any = 0;
    for (long long i = 0; i < tries; ++i)
    {
      const double x = rect.min_x + (static_cast< double >(generator()) - static_cast< double >(minimum)) * x_scale;
      const double y = rect.min_y + (static_cast< double >(generator()) - static_cast< double >(minimum)) * y_scale;
      for (std::size_t j = 0; j < shapes.size(); ++j)
      {
        if (containsData(shapes[j], x, y))
        {
          ++count_in_any;
          break;
        }
      }
    }
    return count_in_any;
  }
}

burukov::Circle::Circle(double radius, double x, double y) noexcept:
  radius_(radius),
  x_(x),
  y_(y)
{}

bool burukov::Circle::contains(double x, double y) const noexcept
{
  const double dx = x - x_;
  const double dy = y - y_;
  return (dx * dx + dy * dy) <= (radius_ * radius_);
}

double burukov::Circle::getMinX() const noexcept
{
  return x_ - radius_;
}

double burukov::Circle::getMinY() const noexcept
{
  return y_ - radius_;
}

double burukov::Circle::getMaxX() const noexcept
{
  return x_ + radius_;
}

double burukov::Circle::getMaxY() const noexcept
{
  return y_ + radius_;
}

void burukov::Circle::rotate(double ksi) noexcept
{
  (void)ksi;
}

void burukov::Circle::print() const
{
  const long long r_val = static_cast< long long >(std::llround(radius_));
  const long long x_val = static_cast< long long >(std::llround(x_));
  const long long y_val = static_cast< long long >(std::llround(y_));
  std::cout << r_val << " (" << x_val << " " << y_val << ")\n";
}

burukov::shape_data_t burukov::Circle::serialize() const noexcept
{
  return {0, radius_, radius_, x_, y_, 0.0};
}

burukov::Ellipse::Ellipse(double a, double b, double x, double y, double angle) noexcept:
  a_(a),
  b_(b),
  x_(x),
  y_(y),
  angle_(angle)
{}

bool burukov::Ellipse::contains(double x, double y) const noexcept
{
  const double dx = x - x_;
  const double dy = y - y_;
  const double cos_a = std::cos(angle_);
  const double sin_a = std::sin(angle_);
  const double local_x = (dx * cos_a + dy * sin_a) / a_;
  const double local_y = (-dx * sin_a + dy * cos_a) / b_;
  return (local_x * local_x + local_y * local_y) <= 1.0;
}

double burukov::Ellipse::getMinX() const noexcept
{
  const double cos_a = std::cos(angle_);
  const double sin_a = std::sin(angle_);
  const double half_w = std::sqrt((a_ * cos_a) * (a_ * cos_a) + (b_ * sin_a) * (b_ * sin_a));
  return x_ - half_w;
}

double burukov::Ellipse::getMinY() const noexcept
{
  const double cos_a = std::cos(angle_);
  const double sin_a = std::sin(angle_);
  const double half_h = std::sqrt((a_ * sin_a) * (a_ * sin_a) + (b_ * cos_a) * (b_ * cos_a));
  return y_ - half_h;
}

double burukov::Ellipse::getMaxX() const noexcept
{
  const double cos_a = std::cos(angle_);
  const double sin_a = std::sin(angle_);
  const double half_w = std::sqrt((a_ * cos_a) * (a_ * cos_a) + (b_ * sin_a) * (b_ * sin_a));
  return x_ + half_w;
}

double burukov::Ellipse::getMaxY() const noexcept
{
  const double cos_a = std::cos(angle_);
  const double sin_a = std::sin(angle_);
  const double half_h = std::sqrt((a_ * sin_a) * (a_ * sin_a) + (b_ * cos_a) * (b_ * cos_a));
  return y_ + half_h;
}

void burukov::Ellipse::rotate(double ksi) noexcept
{
  angle_ += ksi;
}

void burukov::Ellipse::print() const
{
  const long long rx_val = static_cast< long long >(std::llround(a_));
  const long long ry_val = static_cast< long long >(std::llround(b_));
  const long long x_val = static_cast< long long >(std::llround(x_));
  const long long y_val = static_cast< long long >(std::llround(y_));
  const std::ios::fmtflags old_flags = std::cout.flags();
  const std::streamsize old_prec = std::cout.precision();
  std::cout << rx_val << " " << ry_val << " (" << x_val << " " << y_val << ") ";
  std::cout << std::fixed << std::setprecision(3) << angle_ << '\n';
  std::cout.flags(old_flags);
  std::cout.precision(old_prec);
}

burukov::shape_data_t burukov::Ellipse::serialize() const noexcept
{
  return {1, a_, b_, x_, y_, angle_};
}

burukov::rectangle_t burukov::findRectangleRange(const std::vector< std::unique_ptr< Shape > > & shapes)
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

burukov::rectangle_t burukov::findDataRange(const std::vector< shape_data_t > & shapes)
{
  rectangle_t rect = getSingleDataBound(shapes[0]);
  for (std::size_t i = 1; i < shapes.size(); ++i)
  {
    const rectangle_t cur = getSingleDataBound(shapes[i]);
    rect.min_x = std::min(rect.min_x, cur.min_x);
    rect.min_y = std::min(rect.min_y, cur.min_y);
    rect.max_x = std::max(rect.max_x, cur.max_x);
    rect.max_y = std::max(rect.max_y, cur.max_y);
  }
  return rect;
}

long long burukov::parseArgument(const char * arg, const std::string & param)
{
  std::size_t pos = 0;
  const std::string text(arg);
  if (text.empty())
  {
    throw std::invalid_argument("Empty " + param);
  }
  const long long value = std::stoll(text, &pos);
  if (pos != text.length())
  {
    throw std::invalid_argument("Invalid characters in " + param);
  }
  return value;
}

std::pair< long long, long long > burukov::calc(long long tries, long long seed, double min_x, double max_x,
    double min_y, double max_y, const std::vector< std::unique_ptr< Shape > > & shapes)
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

std::pair< double, double > burukov::area(long long threads, long long tries, long long seed,
    const std::vector< std::unique_ptr< Shape > > & shapes, rectangle_t rect)
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
  const double area_intersect = static_cast< double >(total_intersect_hits) / static_cast< double >(tries) * rect_area;

  return std::make_pair(area_union, area_intersect);
}

double burukov::calculateUnionArea(
    long long threads, long long tries, long long seed, const std::vector< shape_data_t > & shapes)
{
  if (shapes.empty())
  {
    return 0.0;
  }

  const rectangle_t rect = findDataRange(shapes);
  const long long actual_threads = (threads > 0) ? threads : 1;
  const long long chunk = tries / actual_threads;
  const long long remainder = tries % actual_threads;

  std::vector< long long > results(static_cast< std::size_t >(actual_threads), 0);
  std::vector< std::thread > thread_pool;
  thread_pool.reserve(static_cast< std::size_t >(actual_threads));

  for (long long j = 0; j < actual_threads; ++j)
  {
    const long long local_seed = seed + j;
    const long long local_tries = (j == actual_threads - 1) ? (chunk + remainder) : chunk;

    thread_pool.emplace_back(
        [&, j, local_tries, local_seed]()
        {
          results[static_cast< std::size_t >(j)] = calcUnionHits(local_tries, local_seed, rect, shapes);
        });
  }

  for (std::size_t i = 0; i < thread_pool.size(); ++i)
  {
    thread_pool[i].join();
  }

  long long total_hits = 0;
  for (std::size_t i = 0; i < results.size(); ++i)
  {
    total_hits += results[i];
  }

  const double rect_area = (rect.max_x - rect.min_x) * (rect.max_y - rect.min_y);
  return static_cast< double >(total_hits) / static_cast< double >(tries) * rect_area;
}
