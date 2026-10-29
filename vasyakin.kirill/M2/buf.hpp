#ifndef BUF_HPP
#define BUF_HPP

#include <string>
#include <streambuf>

namespace vasyakin
{
  class string_buf: public std::streambuf
  {
  public:
    explicit string_buf(const std::string& str)
    {
      char* p = const_cast< char* >(str.data());
      this->setg(p, p, p + str.size());
    }
  };
}

#endif
