#include "help_functions.hpp"

std::vector<std::string> donkeev::splitIntoTokens(const std::string& line)
{
  std::vector<std::string> tokens;
  size_t start = 0;
  size_t end = 0;

  while ((end = line.find(' ', start)) != std::string::npos)
  {
    if (end > start)
    {
      tokens.push_back(line.substr(start, end - start));
    }
    start = end + 1;
  }

  if (start < line.length())
  {
    tokens.push_back(line.substr(start));
  }

  return tokens;
}

bool donkeev::isNumber(const std::string& str)
{
  if (str.empty())
  {
    return false;
  }
  size_t start = 0;
  if (str[0] == '-' || str[0] == '+')
  {
    if (str.length() == 1) return false;
    start = 1;
  }
  for (size_t i = start; i < str.length(); ++i)
  {
    if (!std::isdigit(static_cast<unsigned char>(str[i])))
    {
      return false;
    }
  }
  return true;
}

bool donkeev::isValidCommandTemplate(const std::vector<std::string>& tokens)
{
  if (tokens.empty())
  {
    return false;
  }

  const std::string& cmd = tokens[0];

  if (cmd == "circle")
  {
    return tokens.size() == 5 && isNumber(tokens[2]) && isNumber(tokens[3]) && isNumber(tokens[4]);
  }
  else if (cmd == "set")
  {
    if (tokens.size() < 3 || !isNumber(tokens[2]))
    {
      return false;
    }
    size_t setSize = 0;
    try
    {
      setSize = std::stoul(tokens[2]);
    }
    catch (...)
    {
      return false;
    }
    return tokens.size() == 3 + setSize;
  }
  else if (cmd == "show" || cmd == "showset" || cmd == "frame" || cmd == "frameset")
  {
    return tokens.size() == 2;
  }

  return false;
}
