#include "input.hpp"
#include <cstddef>
#include <stdexcept>

std::vector< std::string > kondrat::splitCommand(const std::string & line)
{
  std::vector< std::string > words;
  size_t i = 0;

  while (i < line.size())
  {
    while (i < line.size() && (line[i] == ' ' || line[i] == '\t'))
    {
      ++i;
    }

    const size_t start = i;

    while (i < line.size() && line[i] != ' ' && line[i] != '\t')
    {
      ++i;
    }

    if (i > start)
    {
      words.emplace_back(line, start, i - start);
    }
  }

  return words;
}

int kondrat::parseInteger(const std::string & word)
{
  size_t position = 0;
  const int value = std::stoi(word, &position);

  if (position != word.size())
  {
    throw std::invalid_argument("Invalid integer");
  }

  return value;
}

bool kondrat::isValidName(const std::string & name)
{
  if (name.empty())
  {
    return false;
  }

  const char first = name.front();

  return (first >= 'a' && first <= 'z') ||
    (first >= 'A' && first <= 'Z');
}
