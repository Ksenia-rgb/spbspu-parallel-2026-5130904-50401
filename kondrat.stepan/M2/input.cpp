#include "input.hpp"
#include <cstddef>

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
