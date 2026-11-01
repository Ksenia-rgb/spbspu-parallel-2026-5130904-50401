#ifndef INPUT_HPP
#define INPUT_HPP

#include <string>
#include <vector>

namespace kondrat
{
  std::vector< std::string > splitCommand(const std::string & line);
  int parseInteger(const std::string & word);
  double parseReal(const std::string & word);
  bool isValidName(const std::string & name);
}

#endif
