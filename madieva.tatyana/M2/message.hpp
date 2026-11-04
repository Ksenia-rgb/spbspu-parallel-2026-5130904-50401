#ifndef MASSAGE_HPP
#define MASSAGE_HPP

#include <string>
#include <vector>
#include "circle.hpp"
#include "parser.hpp"

 namespace madieva
{
  struct AreaRequest
  {
    std::string calc_name;
    long long threads = 0;
    long long tries = 0;
    std::vector< madieva::Circle > circles;
  };

  std::string makeAreaRequest(const std::string & calc_name,
    long long threads, long long tries,
    const std::vector< const Circle * > & figures);
  AreaRequest parseAreaRequest(const Args & args);

  std::string makeAreaResponse(const std::string & calc_name, long long ms, double area);

  void sendAll(int fd, const char * data, std::size_t k);
  void recvAll(int fd, char * data, std::size_t k);
}

#endif
