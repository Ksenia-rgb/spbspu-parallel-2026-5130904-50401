#ifndef CLICKER_HPP
#define CLICKER_HPP

#include <chrono>

namespace vasyakin
{
  class Clicker
  {
  public:
    Clicker():
      start_(std::chrono::steady_clock::now())
    {}

    double millisec() const
    {
      using std::chrono::duration_cast;
      using std::chrono::steady_clock;
      using std::chrono::milliseconds;

      auto t = steady_clock::now();
      return duration_cast< milliseconds >(t - start_).count();
    }

  private:
    std::chrono::time_point< std::chrono::steady_clock > start_;
  };
}

#endif
