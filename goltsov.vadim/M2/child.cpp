#include "child.hpp"
#include <unistd.h>
#include <algorithm>
#include <chrono>
#include <limits>
#include <stdexcept>
#include <vector>
#include <exception>
#include "ipc.hpp"
#include "monte_carlo.hpp"
#include "shared_circles.hpp"

int goltsov::runChild(int rd, int wr, size_t seed)
{
  try
  {
    while (true)
    {
      int err = 0;
      size_t max_count = 0;
      const size_t received = recv(err, rd, reinterpret_cast< char* >(&max_count), sizeof(max_count));
      if (received == 0)
      {
        break;
      }
      if (received != sizeof(max_count))
      {
        throw std::runtime_error("recv failed");
      }

      SharedCircles shared_circles(SHMN, max_count, SharedCircles::Attach{});

      size_t threads = 0;
      recvValue(rd, threads);
      size_t tries = 0;
      recvValue(rd, tries);
      size_t count_circles = 0;
      recvValue(rd, count_circles);

      double min_x = std::numeric_limits< double >::max();
      double max_x = std::numeric_limits< double >::lowest();
      double min_y = std::numeric_limits< double >::max();
      double max_y = std::numeric_limits< double >::lowest();
      std::vector< const Circle* > data;
      for (size_t i = 0; i < count_circles; ++i)
      {
        size_t idxi = 0;
        recvValue(rd, idxi);
        if (idxi >= max_count)
        {
          throw std::runtime_error("invalid index");
        }
        const Circle* circle = shared_circles.getData() + idxi;
        data.push_back(circle);
        min_x = std::min< double >(min_x, circle->x - circle->r);
        max_x = std::max< double >(max_x, circle->x + circle->r);
        min_y = std::min< double >(min_y, circle->y - circle->r);
        max_y = std::max< double >(max_y, circle->y + circle->r);
      }
      auto t0 = std::chrono::steady_clock::now();
      const double res = area(data, threads, tries, seed, min_x, min_y, max_x, max_y);
      auto t1 = std::chrono::steady_clock::now();
      const size_t ms = std::chrono::duration_cast< std::chrono::milliseconds >(t1 - t0).count();
      sendValue(wr, ms);
      sendValue(wr, res);
    }
  }
  catch (const std::exception&)
  {
    close(rd);
    close(wr);
    return 1;
  }
  close(rd);
  close(wr);
  return 0;
}
