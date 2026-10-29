#include "parse.hpp"

#include <cstddef>
#include <stdexcept>

std::vector< std::string > sedov::split(const std::string& line)
{
  std::vector< std::string > words;
  size_t i = 0, n = line.size();
  while (i < n) {
    while (i < n && line[i] == ' ') {
      ++i;
    }
    size_t start = i;
    while (i < n && line[i] != ' ') {
      ++i;
    }
    if (i > start) {
      words.emplace_back(line, start, i - start);
    }
  }
  return words;
}

bool sedov::parseLongLong(const std::string& s, long long& out)
{
  try {
    size_t pos = 0;
    out = std::stoll(s, &pos);
    return pos == s.size();
  } catch (const std::invalid_argument&) {
    return false;
  } catch (const std::out_of_range&) {
    return false;
  }
}
