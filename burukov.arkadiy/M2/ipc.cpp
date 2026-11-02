#include "ipc.hpp"

#include <cerrno>
#include <cstddef>
#include <vector>

#include <unistd.h>

#include "../common/shapes.hpp"

burukov::PipeEndpoint::PipeEndpoint() noexcept:
  fd_(-1)
{}

burukov::PipeEndpoint::PipeEndpoint(int fd) noexcept:
  fd_(fd)
{}

burukov::PipeEndpoint::PipeEndpoint(PipeEndpoint && other) noexcept:
  fd_(other.fd_)
{
  other.fd_ = -1;
}

burukov::PipeEndpoint::~PipeEndpoint() noexcept
{
  closeFd();
}

burukov::PipeEndpoint & burukov::PipeEndpoint::operator=(PipeEndpoint && other) noexcept
{
  if (this != &other)
  {
    closeFd();
    fd_ = other.fd_;
    other.fd_ = -1;
  }
  return *this;
}

int burukov::PipeEndpoint::get() const noexcept
{
  return fd_;
}

void burukov::PipeEndpoint::closeFd() noexcept
{
  if (fd_ >= 0)
  {
    close(fd_);
    fd_ = -1;
  }
}

bool burukov::writeAll(int fd, const void * buffer, std::size_t count) noexcept
{
  const char * const ptr = static_cast< const char * >(buffer);
  std::size_t total = 0;
  while (total < count)
  {
    const ssize_t written = write(fd, ptr + total, count - total);
    if (written < 0)
    {
      if (errno == EINTR)
      {
        continue;
      }
      return false;
    }
    if (written == 0)
    {
      return false;
    }
    total += static_cast< std::size_t >(written);
  }
  return true;
}

bool burukov::readAll(int fd, void * buffer, std::size_t count) noexcept
{
  char * const ptr = static_cast< char * >(buffer);
  std::size_t total = 0;
  while (total < count)
  {
    const ssize_t bytes_read = read(fd, ptr + total, count - total);
    if (bytes_read < 0)
    {
      if (errno == EINTR)
      {
        continue;
      }
      return false;
    }
    if (bytes_read == 0)
    {
      return false;
    }
    total += static_cast< std::size_t >(bytes_read);
  }
  return true;
}

bool burukov::sendTask(int fd, std::size_t calc_id, long long threads, long long tries,
    const std::vector< shape_data_t > & shapes) noexcept
{
  const task_header_t header = {1, calc_id, threads, tries, shapes.size()};
  if (!writeAll(fd, &header, sizeof(header)))
  {
    return false;
  }
  if (!shapes.empty())
  {
    const std::size_t shapes_count = static_cast< std::size_t >(shapes.size());
    const std::size_t bytes = shapes_count * sizeof(shape_data_t);
    if (!writeAll(fd, shapes.data(), bytes))
    {
      return false;
    }
  }
  return true;
}

bool burukov::sendStopSignal(int fd) noexcept
{
  const task_header_t header = {0, 0, 0, 0, 0};
  return writeAll(fd, &header, sizeof(header));
}
