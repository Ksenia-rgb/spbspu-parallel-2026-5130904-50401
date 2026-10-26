#ifndef SHARED_CIRCLES_HPP
#define SHARED_CIRCLES_HPP

#include <cstddef>
#include <ostream>
#include <string>
#include <utility>

namespace goltsov
{
  const char* const SHMN = "/shm-temp";

  struct Circle
  {
    double r;
    double x;
    double y;
  };

  std::ostream& operator<<(std::ostream& os, const Circle& circle);

  class SharedCircles
  {
  public:
    struct Attach
    {};

    SharedCircles(const char* name, const size_t& max_count);
    SharedCircles(const char* name, const size_t& count, Attach);

    SharedCircles(const SharedCircles&) = delete;
    SharedCircles& operator=(const SharedCircles&) = delete;

    ~SharedCircles();

    std::pair< Circle*, bool > new_circle(double r, double x, double y);

    size_t get_size();
    Circle* get_data();
    size_t get_count_circles();

  private:
    std::string name_;
    int shfd_;
    Circle* data_;
    size_t size_;
    size_t max_count_;
    size_t count_circles_ = 0;
    bool owner_;
  };
}

#endif
