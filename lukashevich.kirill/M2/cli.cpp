#include "cli.hpp"

#include <cerrno>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <sys/wait.h>
#include <unistd.h>
#include "calc.hpp"

namespace lukashevich
{
  namespace detail
  {
    void require(bool condition)
    {
      if (!condition)
      {
        throw std::invalid_argument("Invalid command");
      }
    }

    bool isSpace(char value)
    {
      return value == ' ' || value == '\t' || value == '\r' || value == '\v' || value == '\f';
    }

    std::vector< std::string > split(const std::string& line)
    {
      std::vector< std::string > result;
      std::size_t position = 0;
      while (position < line.size())
      {
        if (isSpace(line[position]))
        {
          ++position;
          continue;
        }
        const std::size_t begin = position;
        while (position < line.size() && !isSpace(line[position]))
        {
          ++position;
        }
        result.push_back(line.substr(begin, position - begin));
      }
      return result;
    }

    void checkName(const std::string& name)
    {
      require(!name.empty() && ((name[0] >= 'a' && name[0] <= 'z') || (name[0] >= 'A' && name[0] <= 'Z')));
    }

    long long integer(const std::string& text)
    {
      long long result = 0;
      const char* first = text.data();
      const char* last = first + text.size();
      if (first != last && *first == '+')
      {
        ++first;
        require(first != last && *first >= '0' && *first <= '9');
      }
      const auto parsed = std::from_chars(first, last, result);
      require(parsed.ec == std::errc{} && parsed.ptr == last);
      return result;
    }

    std::size_t count(const std::string& text, bool positive)
    {
      const long long value = integer(text);
      require(positive ? value > 0 : value >= 0);
      require(static_cast< unsigned long long >(value) <= std::numeric_limits< std::size_t >::max());
      return static_cast< std::size_t >(value);
    }

    double angle(const std::string& text)
    {
      char* end = nullptr;
      errno = 0;
      const double value = std::strtod(text.c_str(), &end);
      require(end != text.c_str() && end == text.c_str() + text.size() && errno != ERANGE && std::isfinite(value));
      return value;
    }

    void printShape(std::ostream& output, const Shape& shape)
    {
      output << shape.radius_x;
      if (!shape.circle)
      {
        output << ' ' << shape.radius_y;
      }
      output << " (" << shape.center_x << ' ' << shape.center_y << ')';
      if (!shape.circle)
      {
        output << ' ' << std::fixed << std::setprecision(3) << shape.angle << std::defaultfloat;
      }
      output << '\n';
    }

    void printBounds(std::ostream& output, const Bounds& bounds)
    {
      output << std::defaultfloat << std::setprecision(std::numeric_limits< double >::max_digits10) << '('
             << bounds.min_x << ' ' << bounds.min_y << ") (" << bounds.max_x << ' ' << bounds.max_y << ")\n";
    }

    void printStatus(std::ostream& output, const Calculation& calculation)
    {
      const Response& result = calculation.result;
      require(result.state != CalculationState::failed);
      output << calculation.process << ' ';
      if (result.state == CalculationState::progress)
      {
        output << "<IN PROGRESS>\n";
      }
      else if (result.state == CalculationState::canceled)
      {
        output << "<CANCELED>\n";
      }
      else
      {
        output << std::fixed << std::setprecision(3) << result.milliseconds << ' ' << result.area << std::defaultfloat
               << '\n';
      }
    }

    void reap(pid_t pid) noexcept
    {
      while (waitpid(pid, nullptr, 0) < 0 && errno == EINTR)
      {
      }
    }
  }
}

void lukashevich::CommandLine::addShape(const std::vector< std::string >& arguments, bool circle)
{
  detail::require(arguments.size() == (circle ? 5 : 6));
  const std::string& name = arguments[1];
  detail::checkName(name);
  detail::require(shapes_.count(name) == 0);
  const long long radius_x = detail::integer(arguments[2]);
  const long long radius_y = circle ? radius_x : detail::integer(arguments[3]);
  const std::size_t coordinate = circle ? 3 : 4;
  const long long x = detail::integer(arguments[coordinate]);
  const long long y = detail::integer(arguments[coordinate + 1]);
  detail::require(radius_x > 0 && radius_y > 0);
  shapes_.emplace(name, Shape{radius_x, radius_y, x, y, 0.0, circle});
}

void lukashevich::CommandLine::addSet(const std::vector< std::string >& arguments)
{
  detail::require(arguments.size() >= 3);
  const std::string& name = arguments[1];
  detail::checkName(name);
  detail::require(sets_.count(name) == 0);
  const std::size_t size = detail::count(arguments[2], true);
  detail::require(size == arguments.size() - 3);
  std::vector< std::string > members;
  members.reserve(size);
  for (std::size_t index = 3; index < arguments.size(); ++index)
  {
    detail::require(shapes_.count(arguments[index]) != 0);
    members.push_back(arguments[index]);
  }
  sets_.emplace(name, std::move(members));
}

std::vector< lukashevich::Shape > lukashevich::CommandLine::getSet(const std::string& name) const
{
  const auto& members = sets_.at(name);
  std::vector< Shape > shapes;
  shapes.reserve(members.size());
  for (const std::string& member : members)
  {
    shapes.push_back(shapes_.at(member));
  }
  return shapes;
}

void lukashevich::CommandLine::execute(const std::vector< std::string >& arguments, std::ostream& output)
{
  const std::string& command = arguments[0];
  if (command == "circle")
  {
    addShape(arguments, true);
  }
  else if (command == "set")
  {
    addSet(arguments);
  }
  else if (command == "show" || command == "showset" || command == "frame" || command == "frameset")
  {
    detail::require(arguments.size() == 2);
    if (command == "show")
    {
      detail::printShape(output, shapes_.at(arguments[1]));
    }
    else if (command == "showset")
    {
      for (const Shape& shape : getSet(arguments[1]))
      {
        detail::printShape(output, shape);
      }
    }
    else
    {
      const auto shapes = command == "frame" ? std::vector< Shape >{shapes_.at(arguments[1])} : getSet(arguments[1]);
      detail::printBounds(output, getBounds(shapes));
    }
  }
  else
  {
    output << "<UNKNOWN COMMAND>\n";
  }
}
