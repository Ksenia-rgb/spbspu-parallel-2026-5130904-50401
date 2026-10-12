#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <pthread.h>
#include <random>
#include <string>
#include <vector>

namespace madieva
{

  struct circle_t
  {
    double r;
    double cx;
    double cy;
  };

  struct params_t
  {
    std::size_t threads = 0;
    std::size_t tries = 0;
    std::size_t seed = 0;
  };

  struct frame_t
  {
    double min_x = 0;
    double max_x = 0;
    double min_y = 0;
    double max_y = 0;
  };

  bool parseParams(const int argc, const char * const * argv, params_t & params)
  {
    if (argc != 3 && argc != 4)
    {
      std::cerr << "incorrect number of arguments\n";
      return false;
    }
    try
    {
      if (argv[1][0] == '-')
      {
        std::cerr << "threads must be non-negative\n";
        return false;
      }
      params.threads = static_cast< size_t >(std::stoull(argv[1]));
      if (argv[2][0] == '-')
      {
        std::cerr << "tries must be non-negative\n";
        return false;
      }
      params.tries = static_cast< size_t >(std::stoull(argv[2]));
      if (argc == 4)
      {
        if (argv[3][0] == '-')
        {
          std::cerr << "seed must be non-negative\n";
          return false;
        }
        params.seed = static_cast< size_t >(std::stoull(argv[3]));
      }
    }
    catch (const std::exception & e)
    {
      std::cerr << "cannot parse arguments: " << e.what() << "\n";
      return false;
    }
    if (params.tries == 0)
    {
      std::cerr << "incorrect number of tries\n";
      return false;
    }
    return true;
  }

  bool readCircles(std::vector< circle_t > & circles)
  {
    double r = 0;
    double index = 0;
    double cx = 0;
    double cy = 0;

    while (std::cin >> r >> index >> cx >> cy)
    {
      circles.push_back({r, cx, cy});
    }
    if (!std::cin.eof())
    {
      std::cerr << "error reading figures\n";
      return false;
    }
    if (circles.empty())
    {
      std::cerr << "no figures were provided\n";
      return false;
    }
    return true;
  }

  frame_t computeFrame(const std::vector< circle_t > & circles)
  {
    frame_t frame;
    frame.min_x = circles[0].cx - circles[0].r;
    frame.max_x = circles[0].cx + circles[0].r;
    frame.min_y = circles[0].cy - circles[0].r;
    frame.max_y = circles[0].cy + circles[0].r;
    double min_x = 0;
    double max_x = 0;
    double min_y = 0;
    double max_y = 0;
    for (size_t i = 1; i < circles.size(); ++i)
    {
      min_x = circles[i].cx - circles[i].r;
      max_x = circles[i].cx + circles[i].r;
      min_y = circles[i].cy - circles[i].r;
      max_y = circles[i].cy + circles[i].r;
      if (frame.min_x > min_x)
      {
        frame.min_x = min_x;
      }
      if (frame.max_x < max_x)
      {
        frame.max_x = max_x;
      }
      if (frame.min_y > min_y)
      {
        frame.min_y = min_y;
      }
      if (frame.max_y < max_y)
      {
        frame.max_y = max_y;
      }
    }
    return frame;
  }

  bool isInside(const double x, const double y, const circle_t & circle)
  {
    double a = circle.cx - x;
    double b = circle.cy - y;
    return (a * a + b * b) <= (circle.r * circle.r);
  }

  struct result_t
  {
    double area_all = 0;
    double area_some = 0;
  };

  struct Data
  {
    const std::vector< circle_t > * circles = nullptr;
    frame_t frame;
    std::size_t tries = 0;
    std::size_t seed = 0;

    std::size_t count_all = 0;
    std::size_t count_some = 0;
  };

  void * computeThread(void * data)
  {
    Data * d = static_cast< Data * >(data);
    std::default_random_engine engine(d->seed);
    std::uniform_real_distribution< double > dist_x(d->frame.min_x, d->frame.max_x);
    std::uniform_real_distribution< double > dist_y(d->frame.min_y, d->frame.max_y);
    for (size_t i = 0; i < d->tries; ++i)
    {
      const double x = dist_x(engine);
      const double y = dist_y(engine);

      bool hit_some = false;
      bool hit_all = true;

      for (size_t j = 0; j < d->circles->size(); ++j)
      {
        if (isInside(x, y, (*d->circles)[j]))
        {
          hit_some = true;
        }
        else
        {
          hit_all = false;
        }
      }

      if (hit_some)
      {
        d->count_some++;
      }
      if (hit_all)
      {
        d->count_all++;
      }
    }

    return nullptr;
  }

  bool computeAreas(
      const std::vector< circle_t > & circles, const params_t & params, const frame_t & frame, result_t & result)
  {
    size_t total_all = 0;
    size_t total_some = 0;

    if (params.threads == 0)
    {
      Data data;
      data.circles = &circles;
      data.frame = frame;
      data.tries = params.tries;
      data.seed = params.seed;
      computeThread(&data);
      total_all = data.count_all;
      total_some = data.count_some;
    }
    else
    {
      std::vector< pthread_t > threads;
      std::vector< Data > data;
      try
      {
        threads.resize(params.threads);
        data.resize(params.threads);
      }
      catch (const std::bad_alloc & e)
      {
        std::cerr << "cannot allocate memory for threads: " << e.what() << "\n";
        return false;
      }
      size_t tests_per_thread = params.tries / params.threads;
      size_t remainder = params.tries % params.threads;

      for (size_t i = 0; i < params.threads; ++i)
      {
        data[i].circles = &circles;
        data[i].frame = frame;
        data[i].tries = tests_per_thread;
        if (i == 0)
        {
          data[i].tries += remainder;
        }
        data[i].seed = params.seed + i;
        const int err = pthread_create(&threads[i], nullptr, computeThread, &data[i]);
        if (err != 0)
        {
          std::cerr << "pthread_create failed: " << std::strerror(err) << "\n";
          for (std::size_t j = 0; j < i; ++j)
          {
            pthread_join(threads[j], nullptr);
          }
          return false;
        }
      }
      for (size_t i = 0; i < params.threads; ++i)
      {
        void * ret = nullptr;
        const int err = pthread_join(threads[i], &ret);
        if (err != 0)
        {
          std::cerr << "pthread_join failed: " << std::strerror(err) << "\n";
          return false;
        }
        total_all += data[i].count_all;
        total_some += data[i].count_some;
      }
    }

    const double frame_area = (frame.max_x - frame.min_x) * (frame.max_y - frame.min_y);
    const double scale = frame_area / static_cast< double >(params.tries);

    result.area_all = scale * static_cast< double >(total_all);
    result.area_some = scale * static_cast< double >(total_some);
    return true;
  }
}

int main(int argc, char ** argv)
{
  madieva::params_t params;
  if (!madieva::parseParams(argc, argv, params))
  {
    return 1;
  }
  std::vector< madieva::circle_t > circles;
  if (!madieva::readCircles(circles))
  {
    return 1;
  }
  madieva::frame_t frame = madieva::computeFrame(circles);
  madieva::result_t result{};
  if (!madieva::computeAreas(circles, params, frame, result))
  {
    return 1;
  }
  std::cout << result.area_some << " " << result.area_all << "\n";
  return 0;
}
