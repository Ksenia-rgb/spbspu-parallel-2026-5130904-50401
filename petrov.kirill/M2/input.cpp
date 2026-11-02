#include "input.hpp"

#include <cctype>
#include <cstddef>
#include <stdexcept>

std::vector< std::string > petrov::splitWords(const std::string& line)
{
  std::vector< std::string > words;
  std::string word;
  for (const char symbol: line) {
    if (std::isspace(static_cast< unsigned char >(symbol))) {
      if (!word.empty()) {
        words.push_back(word);
        word.clear();
      }
    } else {
      word += symbol;
    }
  }
  if (!word.empty()) {
    words.push_back(word);
  }
  return words;
}
