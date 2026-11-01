#include "fd.hpp"
#include <unistd.h>

zharov::Fd::Fd(int fd):
  fd_(fd)
{}

zharov::Fd::~Fd()
{
  close(fd_);
}

int zharov::Fd::get() const noexcept
{
  return fd_;
}
