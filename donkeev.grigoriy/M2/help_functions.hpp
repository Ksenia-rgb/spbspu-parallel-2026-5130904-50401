#ifndef HELP_FUNCTIONS_HPP
#define HELP_FUNCTIONS_HPP

#include <vector>
#include <string>

namespace donkeev
{
  std::vector<std::string> splitIntoTokens(const std::string&);
  bool donkeev::isNumber(const std::string&);
  bool donkeev::isValidCommandTemplate(const std::vector<std::string>&);
}

#endif
