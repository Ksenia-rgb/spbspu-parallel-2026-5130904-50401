#ifndef MASSAGE_HPP
#define MASSAGE_HPP

#include <string>
#include <vector>
#include "circle.hpp"

 namespace madieva
{
  std::string makeAreaRequest(const std::string & calc_name,
    long long threads, long long tries,
    const std::vector< const Circle * > & figures);

  void sendAll(int fd, const char * data, std::size_t k);

  void recvAll(int fd, char * data, std::size_t k);
}

#endif
