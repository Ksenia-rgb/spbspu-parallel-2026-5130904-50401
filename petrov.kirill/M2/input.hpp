#ifndef INPUT_HPP
#define INPUT_HPP

#include <string>
#include <vector>

namespace petrov {
  std::vector< std::string > splitWords(const std::string& line);
  bool readInteger(const std::string& word, long long& number);
  bool readReal(const std::string& word, double& number);
  bool isName(const std::string& word);
}

#endif
