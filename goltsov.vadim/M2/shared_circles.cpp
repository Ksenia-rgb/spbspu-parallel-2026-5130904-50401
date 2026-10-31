#include "shared_circles.hpp"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <new>
#include <stdexcept>
#include <ostream>
#include <cstddef>
#include <utility>

std::ostream& goltsov::operator<<(std::ostream& os, const Circle& circle)
{
  return os << circle.r << " (" << circle.x << " " << circle.y << ")";
}

goltsov::SharedCircles::SharedCircles(const char* name, const size_t& max_count)
{
  if (max_count == 0)
  {
    throw std::invalid_argument("max_count must be positive");
  }
  name_ = name;
  max_count_ = max_count;
  owner_ = true;
  shfd_ = shm_open(name, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (shfd_ < 0)
  {
    throw std::runtime_error("shm_open failed");
  }
  size_ = sizeof(Circle) * max_count_;
  if (ftruncate(shfd_, size_) < 0)
  {
    close(shfd_);
    shm_unlink(name);
    throw std::runtime_error("ftruncate failed");
  }
  data_ = static_cast< Circle* >(mmap(NULL, size_, PROT_READ | PROT_WRITE, MAP_SHARED, shfd_, 0));
  if (data_ == MAP_FAILED)
  {
    close(shfd_);
    shm_unlink(name);
    throw std::runtime_error("Map failed");
  }
}

goltsov::SharedCircles::SharedCircles(const char* name, const size_t& count, Attach)
{
  if (count == 0)
  {
    throw std::invalid_argument("count must be positive");
  }
  name_ = name;
  max_count_ = count;
  count_circles_ = count;
  owner_ = false;
  shfd_ = shm_open(name, O_RDWR, S_IRUSR | S_IWUSR);
  if (shfd_ < 0)
  {
    throw std::runtime_error("shm_open failed");
  }
  size_ = sizeof(Circle) * max_count_;
  data_ = static_cast< Circle* >(mmap(NULL, size_, PROT_READ | PROT_WRITE, MAP_SHARED, shfd_, 0));
  if (data_ == MAP_FAILED)
  {
    close(shfd_);
    throw std::runtime_error("Map failed");
  }
}

goltsov::SharedCircles::~SharedCircles()
{
  munmap(data_, size_);
  close(shfd_);
  if (owner_)
  {
    shm_unlink(name_.c_str());
  }
}

std::pair< goltsov::Circle*, bool > goltsov::SharedCircles::newCircle(double r, double x, double y)
{
  bool is_update_size = false;
  if (count_circles_ == max_count_)
  {
    const size_t new_max = max_count_ * 2;
    const size_t new_size = new_max * sizeof(Circle);
    if (ftruncate(shfd_, new_size) < 0)
    {
      throw std::runtime_error("ftruncate failed");
    }
    munmap(data_, size_);
    data_ = static_cast< Circle* >(mmap(NULL, new_size, PROT_READ | PROT_WRITE, MAP_SHARED, shfd_, 0));
    if (data_ == MAP_FAILED)
    {
      throw std::runtime_error("Map failed");
    }
    size_ = new_size;
    max_count_ = new_max;
    is_update_size = true;
  }
  Circle* c = new (data_ + count_circles_) Circle{r, x, y};
  ++count_circles_;
  return std::pair< Circle*, bool >{c, is_update_size};
}

size_t goltsov::SharedCircles::getSize() const
{
  return size_;
}

goltsov::Circle* goltsov::SharedCircles::getData() const
{
  return data_;
}

size_t goltsov::SharedCircles::getCountCircles() const
{
  return count_circles_;
}
