#include "commands.hpp"
#include "parse.hpp"

bool zhuravleva::addCircle(Storage& storage, const std::vector< std::string >& args)
{
  if (args.size() != 5)
  {
    return false;
  }
  int radius = 0;
  int x = 0;
  int y = 0;
  if (!parseInt(args[2], radius) || !parseInt(args[3], x) ||
    !parseInt(args[4], y) || radius <= 0 || args[1].empty())
  {
    return false;
  }
  const Shape shape{ShapeType::circle, radius, radius, x, y, 0.0};
  return storage.shapes.emplace(args[1], shape).second;
}

bool zhuravleva::addEllipse(Storage& storage, const std::vector< std::string >& args)
{
  if (args.size() != 6)
  {
    return false;
  }
  int rx = 0;
  int ry = 0;
  int x = 0;
  int y = 0;

  if (!parseInt(args[2], rx) || !parseInt(args[3], ry) ||
    !parseInt(args[4], x) || !parseInt(args[5], y) ||
    rx <= 0 || ry <= 0 || args[1].empty())
  {
    return false;
  }
  const Shape shape{ShapeType::ellipse, rx, ry, x, y, 0.0};
  return storage.shapes.emplace(args[1], shape).second;
}

bool zhuravleva::rotateShape(Storage& storage, const std::vector< std::string >& args)
{
  if (args.size() != 3)
  {
    return false;
  }

  double angle = 0.0;
  if (!parseDouble(args[2], angle))
  {
    return false;
  }

  auto it = storage.shapes.find(args[1]);
  if (it == storage.shapes.end())
  {
    return false;
  }
  it->second.angle += angle;
  return true;
}
