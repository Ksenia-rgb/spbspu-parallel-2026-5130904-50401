#ifndef FD_HPP
#define FD_HPP

namespace zharov
{
  class Fd
  {
  public:
    explicit Fd(int fd);
    ~Fd();
    Fd(const Fd&) = delete;
    Fd& operator=(const Fd&) = delete;
    int get() const noexcept;

  private:
    int fd_;
  };
}

#endif
