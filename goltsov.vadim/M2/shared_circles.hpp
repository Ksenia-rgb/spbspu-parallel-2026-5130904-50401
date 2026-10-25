#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>

namespace goltsov
{
  struct Circle
  {
    size_t r;
    long long x;
    long long y;

    friend std::ostream& operator<<(std::ostream& os, const Circle& circle)
    {
      os << circle.r << " (" << circle.x << " " << circle.y << ")";
    }
  };
  
  class SharedCircles
  {
  public:
    SharedCircles(const char* name)
    {
      name_ = name;
      shfd_ = shm_open(name, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
      if (shfd_ < 0)
      {
        throw std::runtime_error("shm_open failed");
      }
      max_count_ = 10;
      size_ = sizeof(Circle) * max_count_;
      if (ftruncate(shfd_, size_) < 0)
      {
        close(shfd_);
        shm_unlink(name);
        throw std::runtime_error("ftruncate failed");
      }
      data_ = (Circle*)mmap(NULL, size_, PROT_READ | PROT_WRITE, MAP_SHARED, shfd_, 0);
      if (data_ == MAP_FAILED)
      {
        close(shfd_);
        shm_unlink(name);
        throw std::runtime_error("Map failed");
      }
    }
  
    SharedCircles(const SharedCircles&) = delete;
    SharedCircles& operator=(const SharedCircles&) = delete;
  
    ~SharedCircles()
    {
      munmap(data_, size_);
      close(shfd_);
      shm_unlink(name_.c_str());
    }
  
    std::pair< Circle*, bool > new_circle(size_t r, long long x, long long y)
    {
      bool is_update_size = false;
      if (count_circles_ == max_count_)
      {
        size_t new_max = max_count_ * 2;
        size_t new_size = new_max * sizeof(Circle);
        if (ftruncate(shfd_, new_size) < 0)
        {
          throw std::runtime_error("ftruncate failed");
        }
        munmap(data_, size_);
        data_ = (Circle*)mmap(NULL, new_size, PROT_READ | PROT_WRITE, MAP_SHARED, shfd_, 0);
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
  
    size_t get_size()
    {
      return size_;
    }

    Circle* get_data()
    {
      return data_;
    }

    size_t get_count_circles()
    {
      return count_circles_;
    }

  private:
    std::string name_;
    int shfd_;
    Circle* data_;
    size_t size_;
    size_t max_count_;
    size_t count_circles_ = 0;
  };
}
