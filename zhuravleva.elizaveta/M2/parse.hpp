#ifndef PARSE_HPP
#define PARSE_HPP

#include <string>
#include <vector>

namespace zhuravleva
{
  std::vector< std::string > splitLine(const std::string& line);
  bool parseInt(const std::string& text, int& value);
}

#endif
