#include "parser.hpp"

#include <string>
#include <stdexcept>

madieva::Args madieva::split(const std::string & line)
{
  const std::string delimiters = " \n\r\t";
  Args args;
  auto pos_start = line.find_first_not_of(delimiters, 0);
  while (pos_start != std::string::npos)
  {
    auto pos_end = line.find_first_of(delimiters, pos_start);
    if (pos_end == std::string::npos)
    {
      args.push_back(line.substr(pos_start, line.size() - pos_start));
    }
    else
    {
      args.push_back(line.substr(pos_start, pos_end - pos_start));
    }
    pos_start = line.find_first_not_of(delimiters, pos_end);
  }
  return args;
}

long long madieva::parseInteger(const std::string & arg)
{
  std::size_t pos = 0;
  long long res = 0;
  try
  {
    res = std::stoll(arg, &pos);
  }
  catch (const std::out_of_range &)
  {
    throw std::invalid_argument("the number is too large");
  }
  if (pos != arg.size())
  {
    throw std::invalid_argument("incorrect number");
  }
  return res;
}
