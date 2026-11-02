#include "commands.hpp"
#include <cctype>
#include <iostream>
#include <stdexcept>
#include "calc_manager.hpp"

namespace
{
  bool parseLongLong(const std::string& s, long long& out)
  {
    if (s.empty()) {
      return false;
    }
    try {
      size_t pos = 0;
      out = std::stoll(s, &pos);
      return pos == s.size();
    } catch (...) {
      return false;
    }
  }
}

std::vector< std::string > zubarev::splitLine(const std::string& line)
{
  std::vector< std::string > tokens;
  size_t start = 0;

  while (start < line.size()) {
    while (start < line.size() && std::isspace(static_cast< unsigned char >(line[start]))) {
      ++start;
    }
    if (start >= line.size()) {
      break;
    }
    size_t end = start;
    while (end < line.size() && !std::isspace(static_cast< unsigned char >(line[end]))) {
      ++end;
    }
    tokens.push_back(line.substr(start, end - start));
    start = end;
  }

  return tokens;
}

bool zubarev::isValidName(const std::string& name)
{
  if (name.empty()) {
    return false;
  }
  unsigned char first = static_cast< unsigned char >(name[0]);
  if (!((first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z'))) {
    return false;
  }
  for (char ch : name) {
    if (std::isspace(static_cast< unsigned char >(ch))) {
      return false;
    }
  }
  return true;
}

bool zubarev::isNameOccupied(const ShapesTable& table, const std::string& name)
{
  return (table.circles.find(name) != table.circles.end()) || (table.sets.find(name) != table.sets.end());
}

void zubarev::cmd_circle(std::istream& in, std::ostream&, ShapesTable& shapes)
{
  std::string line;
  std::getline(in, line);
  std::vector< std::string > tokens = splitLine(line);

  if (tokens.size() != 4) {
    throw std::logic_error("invalid circle arguments count");
  }

  const std::string& name = tokens[0];
  if (!isValidName(name) || isNameOccupied(shapes, name)) {
    throw std::logic_error("invalid or occupied circle name");
  }

  long long r = 0;
  long long x = 0;
  long long y = 0;

  if (!parseLongLong(tokens[1], r) || !parseLongLong(tokens[2], x) || !parseLongLong(tokens[3], y)) {
    throw std::logic_error("circle parameters must be integers");
  }

  if (r <= 0) {
    throw std::logic_error("circle radius must be positive");
  }

  shapes.circles[name] = Circle(static_cast< double >(x), static_cast< double >(y), static_cast< double >(r));
}

void zubarev::cmd_set(std::istream& in, std::ostream&, ShapesTable& shapes)
{
  std::string line;
  std::getline(in, line);
  std::vector< std::string > tokens = splitLine(line);

  if (tokens.size() < 2) {
    throw std::logic_error("invalid set arguments count");
  }

  const std::string& name = tokens[0];
  if (!isValidName(name) || isNameOccupied(shapes, name)) {
    throw std::logic_error("invalid or occupied set name");
  }

  long long count = 0;
  if (!parseLongLong(tokens[1], count) || count <= 0) {
    throw std::logic_error("set size must be positive integer");
  }

  if (tokens.size() != 2 + static_cast< size_t >(count)) {
    throw std::logic_error("set size does not match shapes count");
  }

  std::vector< std::string > set_shapes;
  set_shapes.reserve(count);

  for (size_t i = 2; i < tokens.size(); ++i) {
    const std::string& shape_name = tokens[i];
    if (shapes.circles.find(shape_name) == shapes.circles.end()) {
      throw std::logic_error("shape does not exist: " + shape_name);
    }
    set_shapes.push_back(shape_name);
  }

  shapes.sets[name] = std::move(set_shapes);
}

void zubarev::cmd_show(std::istream& in, std::ostream& out, ShapesTable& shapes)
{
  std::string line;
  std::getline(in, line);
  std::vector< std::string > tokens = splitLine(line);

  if (tokens.size() != 1) {
    throw std::logic_error("invalid show arguments count");
  }

  auto it = shapes.circles.find(tokens[0]);
  if (it == shapes.circles.end()) {
    throw std::logic_error("shape not found: " + tokens[0]);
  }

  const Circle& c = it->second;
  out << static_cast< long long >(c.r) << " (" << static_cast< long long >(c.x) << " " << static_cast< long long >(c.y)
      << ")\n";
}

void zubarev::cmd_showset(std::istream& in, std::ostream& out, ShapesTable& shapes)
{
  std::string line;
  std::getline(in, line);
  std::vector< std::string > tokens = splitLine(line);

  if (tokens.size() != 1) {
    throw std::logic_error("invalid showset arguments count");
  }

  auto it = shapes.sets.find(tokens[0]);
  if (it == shapes.sets.end()) {
    throw std::logic_error("set not found: " + tokens[0]);
  }

  for (const auto& shape_name : it->second) {
    auto shape_it = shapes.circles.find(shape_name);
    if (shape_it == shapes.circles.end()) {
      throw std::logic_error("shape not found in set: " + shape_name);
    }
    const Circle& c = shape_it->second;
    out << static_cast< long long >(c.r) << " (" << static_cast< long long >(c.x) << " "
        << static_cast< long long >(c.y) << ")\n";
  }
}

void zubarev::cmd_frame(std::istream& in, std::ostream& out, ShapesTable& shapes)
{
  std::string line;
  std::getline(in, line);
  std::vector< std::string > tokens = splitLine(line);

  if (tokens.size() != 1) {
    throw std::logic_error("invalid frame arguments count");
  }

  auto it = shapes.circles.find(tokens[0]);
  if (it == shapes.circles.end()) {
    throw std::logic_error("shape not found: " + tokens[0]);
  }

  Frame f = frame(it->second);
  out << "(" << static_cast< long long >(f.min_x) << " " << static_cast< long long >(f.min_y) << ") ("
      << static_cast< long long >(f.max_x) << " " << static_cast< long long >(f.max_y) << ")\n";
}

void zubarev::cmd_frameset(std::istream& in, std::ostream& out, ShapesTable& shapes)
{
  std::string line;
  std::getline(in, line);
  std::vector< std::string > tokens = splitLine(line);

  if (tokens.size() != 1) {
    throw std::logic_error("invalid frameset arguments count");
  }

  auto it = shapes.sets.find(tokens[0]);
  if (it == shapes.sets.end()) {
    throw std::logic_error("set not found: " + tokens[0]);
  }

  std::vector< Circle > set_circles;
  set_circles.reserve(it->second.size());

  for (const auto& shape_name : it->second) {
    auto shape_it = shapes.circles.find(shape_name);
    if (shape_it == shapes.circles.end()) {
      throw std::logic_error("shape not found in set: " + shape_name);
    }
    set_circles.push_back(shape_it->second);
  }

  Frame f = frame(set_circles);
  out << "(" << static_cast< long long >(f.min_x) << " " << static_cast< long long >(f.min_y) << ") ("
      << static_cast< long long >(f.max_x) << " " << static_cast< long long >(f.max_y) << ")\n";
}

void zubarev::processCommand(const std::string& line, ShapesTable& shapes, CalcTable& calcs, std::ostream& out)
{
  std::vector< std::string > tokens = splitLine(line);
  if (tokens.empty()) {
    return;
  }

  const std::string& cmd = tokens[0];

  try
  {
    if (cmd == "circle") {
      if (tokens.size() != 5) {
        throw std::logic_error("invalid circle arguments count");
      }
      const std::string& name = tokens[1];
      if (!isValidName(name) || isNameOccupied(shapes, name)) {
        throw std::logic_error("invalid or occupied circle name");
      }
      long long r = 0, x = 0, y = 0;
      if (!parseLongLong(tokens[2], r) || !parseLongLong(tokens[3], x) || !parseLongLong(tokens[4], y)) {
        throw std::logic_error("circle parameters must be integers");
      }
      if (r <= 0) {
        throw std::logic_error("circle radius must be positive");
      }
      shapes.circles[name] = Circle(static_cast< double >(x), static_cast< double >(y), static_cast< double >(r));
    }
    else if (cmd == "set") {
      if (tokens.size() < 3) {
        throw std::logic_error("invalid set arguments count");
      }
      const std::string& name = tokens[1];
      if (!isValidName(name) || isNameOccupied(shapes, name)) {
        throw std::logic_error("invalid or occupied set name");
      }
      long long count = 0;
      if (!parseLongLong(tokens[2], count) || count <= 0) {
        throw std::logic_error("set size must be positive integer");
      }
      if (tokens.size() != 3 + static_cast< size_t >(count)) {
        throw std::logic_error("set size does not match shapes count");
      }
      std::vector< std::string > set_shapes;
      set_shapes.reserve(count);
      for (size_t i = 3; i < tokens.size(); ++i) {
        const std::string& shape_name = tokens[i];
        if (shapes.circles.find(shape_name) == shapes.circles.end()) {
          throw std::logic_error("shape does not exist: " + shape_name);
        }
        set_shapes.push_back(shape_name);
      }
      shapes.sets[name] = std::move(set_shapes);
    }
    else if (cmd == "show") {
      if (tokens.size() != 2) {
        throw std::logic_error("invalid show arguments count");
      }
      auto it = shapes.circles.find(tokens[1]);
      if (it == shapes.circles.end()) {
        throw std::logic_error("shape not found: " + tokens[1]);
      }
      const Circle& c = it->second;
      out << static_cast< long long >(c.r) << " (" << static_cast< long long >(c.x) << " " << static_cast< long long >(c.y) << ")\n";
    }
    else if (cmd == "showset") {
      if (tokens.size() != 2) {
        throw std::logic_error("invalid showset arguments count");
      }
      auto it = shapes.sets.find(tokens[1]);
      if (it == shapes.sets.end()) {
        throw std::logic_error("set not found: " + tokens[1]);
      }
      for (const auto& shape_name : it->second) {
        auto shape_it = shapes.circles.find(shape_name);
        if (shape_it == shapes.circles.end()) {
          throw std::logic_error("shape not found in set: " + shape_name);
        }
        const Circle& c = shape_it->second;
        out << static_cast< long long >(c.r) << " (" << static_cast< long long >(c.x) << " " << static_cast< long long >(c.y) << ")\n";
      }
    }
    else if (cmd == "frame") {
      if (tokens.size() != 2) {
        throw std::logic_error("invalid frame arguments count");
      }
      auto it = shapes.circles.find(tokens[1]);
      if (it == shapes.circles.end()) {
        throw std::logic_error("shape not found: " + tokens[1]);
      }
      Frame f = frame(it->second);
      out << "(" << static_cast< long long >(f.min_x) << " " << static_cast< long long >(f.min_y) << ") ("
          << static_cast< long long >(f.max_x) << " " << static_cast< long long >(f.max_y) << ")\n";
    }
    else if (cmd == "frameset") {
      if (tokens.size() != 2) {
        throw std::logic_error("invalid frameset arguments count");
      }
      auto it = shapes.sets.find(tokens[1]);
      if (it == shapes.sets.end()) {
        throw std::logic_error("set not found: " + tokens[1]);
      }
      std::vector< Circle > set_circles;
      set_circles.reserve(it->second.size());
      for (const auto& shape_name : it->second) {
        auto shape_it = shapes.circles.find(shape_name);
        if (shape_it == shapes.circles.end()) {
          throw std::logic_error("shape not found in set: " + shape_name);
        }
        set_circles.push_back(shape_it->second);
      }
      Frame f = frame(set_circles);
      out << "(" << static_cast< long long >(f.min_x) << " " << static_cast< long long >(f.min_y) << ") ("
          << static_cast< long long >(f.max_x) << " " << static_cast< long long >(f.max_y) << ")\n";
    }
    else if (cmd == "spawn") {
      if (tokens.size() != 3) {
        throw std::logic_error("invalid spawn arguments count");
      }
      const std::string& process_name = tokens[1];
      long long seed = 0;
      if (!parseLongLong(tokens[2], seed) || seed < 0) {
        throw std::logic_error("invalid seed");
      }
      spawn(calcs, process_name, static_cast< size_t >(seed));
    }
    else if (cmd == "area-on") {
      if (tokens.size() != 6) {
        throw std::logic_error("invalid area-on arguments count");
      }
      const std::string& process_name = tokens[1];
      const std::string& calc_name = tokens[2];
      const std::string& set_name = tokens[3];
      long long threads = 0;
      long long tries = 0;
      if (!parseLongLong(tokens[4], threads) || threads < 0) {
        throw std::logic_error("invalid threads count");
      }
      if (!parseLongLong(tokens[5], tries) || tries <= 0) {
        throw std::logic_error("invalid tries count");
      }
      auto set_it = shapes.sets.find(set_name);
      if (set_it == shapes.sets.end()) {
        throw std::logic_error("set not found: " + set_name);
      }
      std::vector< Circle > set_circles;
      set_circles.reserve(set_it->second.size());
      for (const auto& shape_name : set_it->second) {
        auto shape_it = shapes.circles.find(shape_name);
        if (shape_it == shapes.circles.end()) {
          throw std::logic_error("shape not found in set: " + shape_name);
        }
        set_circles.push_back(shape_it->second);
      }
      uint64_t actual_threads = (threads == 0) ? 1 : static_cast< uint64_t >(threads);
      areaOn(calcs, process_name, calc_name, set_circles, actual_threads, static_cast< uint64_t >(tries));
    }
    else if (cmd == "status") {
      if (tokens.size() != 2) {
        throw std::logic_error("invalid status arguments count");
      }
      status(calcs, tokens[1], out);
    }
    else if (cmd == "wait") {
      if (tokens.size() != 2) {
        throw std::logic_error("invalid wait arguments count");
      }
      wait(calcs, tokens[1], out);
    }
    else {
      out << "<UNKNOWN COMMAND>\n";
    }
  }
  catch (const std::exception&)
  {
    out << "<INVALID COMMAND>\n";
  }
}
