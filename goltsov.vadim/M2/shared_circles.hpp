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

    std::pair< Circle*, bool > newCircle(double r, double x, double y);

    size_t getSize() const;
    Circle* getData() const;
    size_t getCountCircles() const;

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
