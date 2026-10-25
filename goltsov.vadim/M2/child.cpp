#include <iostream>
#include <vector>
#include <tuple>
#include <random>
#include <future>
#include <utility>
#include <cstddef>
#include <string>
#include <algorithm>
#include <stdexcept>
#include "shared_circles.hpp"

namespace goltsov
{
  const char* const SHMN = "shm-temp";

  size_t send(int& err, int wr, const char* b, size_t k) {
    size_t r = 0;
    while (r < k) {
      err = write(wr, b + r, k - r);
      if (err < 0) break;
      r += err;
    }
    return r;
  }

  size_t recv(int& err, int rd, char* b, size_t k) {
    size_t r = 0;
    while (r < k) {
      err = read(rd, b + r, k - r);
      if (err <= 0) break;
      r += err;
    }
    return r;
  }

  bool isInside(Circle* s, const double& x, const double& y);

  size_t calc(const std::vector< Circle* >& data, const size_t& tests,
      const size_t& seed, const double& min_x, const double& min_y, const double& max_x, const double& max_y);

  double area(const std::vector< Circle* >& data,
      const size_t& threads, const size_t& tries, const size_t& start_seed, const double& min_x, const double& min_y,
      const double& max_x, const double& max_y);
}

int main(int argc, char** argv)
{
  int rd = std::stoi(argv[1]);
  int wr = std::stoi(argv[2]);
  size_t seed = std::stoi(argv[3]);
  int err = 0;
  size_t max_count;
  goltsov::recv(err, rd, (char*)&max_count, sizeof(max_count));

  goltsov::SharedCircles shared_circles = goltsov::SharedCircles(goltsov::SHMN, max_count);

  size_t threads;
  goltsov::recv(err, rd, (char*)&threads, sizeof(threads));
  size_t tries;
  goltsov::recv(err, rd, (char*)&tries, sizeof(tries));
  size_t count_circles;
  goltsov::recv(err, rd, (char*)&count_circles, sizeof(count_circles));

  double min_x = std::numeric_limits< double >::max();
  double max_x = std::numeric_limits< double >::min();
  double min_y = std::numeric_limits< double >::max();
  double max_y = std::numeric_limits< double >::min();
  std::vector< goltsov::Circle* > data;
  for (size_t i = 0; i < count_circles; ++i)
  {
    size_t idxi;
    goltsov::recv(err, rd, (char*)&idxi, sizeof(idxi));
    goltsov::Circle* circle = shared_circles.get_data() + idxi;
    data.push_back(circle);
    min_x = std::min< double >(min_x, circle->x - circle->r);
    max_x = std::max< double >(max_x, circle->x + circle->r);
    min_y = std::min< double >(min_y, circle->y - circle->r);
    max_y = std::max< double >(max_y, circle->y + circle->r);
  }
  double res = goltsov::area(data, threads, tries, seed, min_x, min_y, max_x, max_y);
  goltsov::send(err, wr, (char*)&res, sizeof(res));
}

bool goltsov::isInside(Circle* s, const double& x, const double& y)
{
  const double r = s->r;
  const double sx = s->x;
  const double sy = s->y;
  return r * r >= (x - sx) * (x - sx) + (y - sy) * (y - sy);
}

size_t goltsov::calc(const std::vector< Circle* >& data,
    const size_t& tests, const size_t& seed, const double& min_x, const double& min_y, const double& max_x,
    const double& max_y)
{
  std::default_random_engine engine(seed);
  std::uniform_real_distribution< double > dist_x(min_x, max_x);
  std::uniform_real_distribution< double > dist_y(min_y, max_y);

  size_t count_inside = 0;
  size_t count_inside_in_all = 0;

  for (size_t i = 0; i < tests; ++i)
  {
    const double x = dist_x(engine);
    const double y = dist_y(engine);
    bool is_inside = false;

    for (size_t j = 0; j < data.size(); ++j)
    {
      count_inside++;
      break;
    }

  }
  return count_inside;
}

double goltsov::area(const std::vector< Circle* >& data,
    const size_t& threads, const size_t& tries, const size_t& start_seed, const double& min_x, const double& min_y,
    const double& max_x, const double& max_y)
{
  std::vector< std::future< size_t > > results_in_threads(threads);

  for (size_t i = 0; i < tries % threads; ++i)
  {
    results_in_threads[i]
        = std::async(std::launch::async, calc, data, tries / threads + 1, start_seed + i, min_x, min_y, max_x, max_y);
  }
  for (size_t i = tries % threads; i < threads; ++i)
  {
    results_in_threads[i]
        = std::async(std::launch::async, calc, data, tries / threads, start_seed + i, min_x, min_y, max_x, max_y);
  }

  size_t count_inside = 0;
  size_t count_inside_in_all = 0;

  for (size_t i = 0; i < threads; ++i)
  {
    count_inside += results_in_threads[i].get();
  }

  return (max_x - min_x) * (max_y - min_y) * (static_cast< double >(count_inside) / static_cast< double >(tries));
}
