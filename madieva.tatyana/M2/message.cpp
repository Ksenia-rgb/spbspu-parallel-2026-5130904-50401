#include "message.hpp"

#include <string>

std::string madieva::makeAreaRequest(const std::string & calc_name,
  long long threads, long long tries,
  const std::vector< const Circle * > & figures)
{
  std::string message;
  message += calc_name;
  message += ' ';
  message += std::to_string(threads);
  message += ' ';
  message += std::to_string(tries);
  message += ' ';
  message += std::to_string(figures.size());
  message += ' ';
  for (const Circle * circle : figures)
  {
    message += std::to_string(circle->getRadius());
    message += ' ';
    message += std::to_string(circle->getX());
    message += ' ';
    message += std::to_string(circle->getY());
    message += ' ';
  }
  return message;
}


void madieva::sendAll(int fd, const char * data, std::size_t k)
{
  std::size_t sent = 0;
  while (sent < k)
  {
    const int r = ::write(fd, data + sent, k - sent);
    if (r < 0)
    {
      if (errno == EINTR)
      {
        continue;
      }
      throw std::runtime_error(std::string("write failed: ") + std::strerror(errno));
    }
    sent += static_cast< std::size_t >(r);
  }
}

void madieva::recvAll(int fd, char * data, std::size_t k)
{
  std::size_t received = 0;
  while (received < k)
  {
    const int r = ::read(fd, data + received, k - received);
    if (r < 0)
    {
      if (errno == EINTR)
      {
        continue;
      }
      throw std::runtime_error(std::string("read failed: ") + std::strerror(errno));
    }
    if (r == 0)
    {
      throw std::runtime_error("unexpected EOF");
    }
    received += static_cast< std::size_t >(r);
  }
}