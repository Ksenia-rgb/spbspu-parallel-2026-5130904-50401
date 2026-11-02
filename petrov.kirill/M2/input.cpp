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

bool petrov::readInteger(const std::string& word, long long& number)
{
  size_t length = 0;
  try {
    number = std::stoll(word, &length);
  } catch (const std::logic_error&) {
    return false;
  }
  return length == word.size();
}

bool petrov::readReal(const std::string& word, double& number)
{
  size_t length = 0;
  try {
    number = std::stod(word, &length);
  } catch (const std::logic_error&) {
    return false;
  }
  return length == word.size();
}

bool petrov::isName(const std::string& word)
{
  if (word.empty()) {
    return false;
  }
  const char first = word[0];
  const bool is_small_letter = (first >= 'a') && (first <= 'z');
  const bool is_big_letter = (first >= 'A') && (first <= 'Z');
  return is_small_letter || is_big_letter;
}
