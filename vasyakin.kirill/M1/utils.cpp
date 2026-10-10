#include "utils.hpp"

#include <stdexcept>
#include <random>
#include <thread>
#include <algorithm>
#include <cstddef>
#include <vector>
#include <string>
#include <utility>
#include <memory>

vasyakin::Circle::Circle(double radius, double x, double y) noexcept:
  radius_(radius),
  x_(x),
  y_(y)
{}

bool vasyakin::Circle::contains(double dx, double dy) const noexcept
{
  return (dx - x_) * (dx - x_) + (dy - y_) * (dy - y_) <= radius_ * radius_;
}

double vasyakin::Circle::getMinX() const noexcept
{
  return x_ - radius_;
}

double vasyakin::Circle::getMinY() const noexcept
{
  return y_ - radius_;
}

double vasyakin::Circle::getMaxX() const noexcept
{
  return x_ + radius_;
}

double vasyakin::Circle::getMaxY() const noexcept
{
  return y_ + radius_;
}

vasyakin::Ellipse::Ellipse(double a, double b, double x, double y) noexcept:
  a_(a),
  b_(b),
  x_(x),
  y_(y)
{}

bool vasyakin::Ellipse::contains(double dx, double dy) const noexcept
{
  const double dx_norm = (dx - x_) / a_;
  const double dy_norm = (dy - y_) / b_;

  return dx_norm * dx_norm + dy_norm * dy_norm <= 1.0;
}

double vasyakin::Ellipse::getMinX() const noexcept
{
  return x_ - a_;
}

double vasyakin::Ellipse::getMinY() const noexcept
{
  return y_ - b_;
}

double vasyakin::Ellipse::getMaxX() const noexcept
{
  return x_ + a_;
}

double vasyakin::Ellipse::getMaxY() const noexcept
{
  return y_ + b_;
}

vasyakin::rectangle_t vasyakin::findRectangleRange(const std::vector< std::unique_ptr< vasyakin::Figure > >& figures)
{
  vasyakin::rectangle_t rect
      = {figures[0]->getMinX(), figures[0]->getMinY(), figures[0]->getMaxX(), figures[0]->getMaxY()};

  for (size_t i = 1; i < figures.size(); ++i)
  {
    rect.min_x = std::min(rect.min_x, figures[i]->getMinX());
    rect.min_y = std::min(rect.min_y, figures[i]->getMinY());
    rect.max_x = std::max(rect.max_x, figures[i]->getMaxX());
    rect.max_y = std::max(rect.max_y, figures[i]->getMaxY());
  }

  return rect;
}

long long vasyakin::parseArgument(const char* arg, const std::string& param)
{
  size_t pos = 0;
  const long long value = std::stoll(arg, &pos);

  if (pos != std::string(arg).length())
  {
    throw std::invalid_argument("Invalid characters in " + param);
  }

  return value;
}

std::pair< long long, long long > vasyakin::calc(long long tries, long long seed, double min_x, double max_x,
    double min_y, double max_y, const std::vector< std::unique_ptr< vasyakin::Figure > >& figures)
{
  long long count_in_one_circle = 0;
  long long count_in_all_circles = 0;

  std::default_random_engine engine(static_cast< unsigned int >(seed));
  std::uniform_real_distribution< double > dist_x(min_x, max_x);
  std::uniform_real_distribution< double > dist_y(min_y, max_y);

  for (long long i = 0; i < tries; ++i)
  {
    const double x = dist_x(engine);
    const double y = dist_y(engine);

    bool in_any = false;
    bool in_all = true;

    for (size_t j = 0; j < figures.size(); ++j)
    {
      if (figures[j]->contains(x, y))
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
      ++count_in_one_circle;
    }

    if (in_all)
    {
      ++count_in_all_circles;
    }
  }

  return std::make_pair(count_in_one_circle, count_in_all_circles);
}

std::pair< double, double > vasyakin::area(long long threads, long long tries, long long seed,
    const std::vector< std::unique_ptr< vasyakin::Figure > >& figures, vasyakin::rectangle_t rect)
{
  long long local_tries = tries;

  const long long chunk = tries / threads;
  const long long remainder = tries % threads;

  std::vector< std::pair< long long, long long > > results(threads);

  std::vector< std::thread > thread_pool;
  thread_pool.reserve(threads);

  for (long long j = 0; j < threads; ++j)
  {
    const long long local_seed = seed + j;
    local_tries = (j == threads - 1) ? chunk + remainder : chunk;

    thread_pool.emplace_back(
        [&, j, local_tries, local_seed]()
        {
          results[j] = vasyakin::calc(local_tries, local_seed, rect.min_x, rect.max_x, rect.min_y, rect.max_y, figures);
        });
  }

  for (size_t i = 0; i < thread_pool.size(); ++i)
  {
    thread_pool[i].join();
  }

  long long total_union_hits = 0;
  long long total_intersect_hits = 0;

  for (size_t i = 0; i < results.size(); ++i)
  {
    total_union_hits += results[i].first;
    total_intersect_hits += results[i].second;
  }

  const double rect_area = (rect.max_x - rect.min_x) * (rect.max_y - rect.min_y);

  const double area_union = static_cast< double >(total_union_hits) / static_cast< double >(tries) * rect_area;
  const double area_intersect = static_cast< double >(total_intersect_hits) / static_cast< double >(tries) * rect_area;

  return std::make_pair(area_union, area_intersect);
}
