#ifndef BUF_HPP
#define BUF_HPP

#include <string>
#include <streambuf>

namespace vasyakin
{
  class StringBuf: public std::streambuf
  {
  public:
    explicit StringBuf(const std::string& str)
    {
      const char* const p = const_cast< char* >(str.data());
      this->setg(const_cast< char* >(p), const_cast< char* >(p), const_cast< char* >(p) + str.size());
    }
  };
}

#endif
