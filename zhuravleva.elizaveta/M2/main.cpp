#include "commands.hpp"
#include "parse.hpp"
#include <iostream>
#include <string>
#include <vector>

int main()
{
  zhuravleva::Storage storage;
  std::string line;

  while (std::getline(std::cin, line))
  {
    const std::vector< std::string > args = zhuravleva::splitLine(line);
    if (args.empty())
    {
      continue;
    }
    bool valid = false;
    if (args[0] == "circle")
    {
      valid = zhuravleva::addCircle(storage, args);
    }
    else if (args[0] == "ellipse")
    {
      valid = zhuravleva::addEllipse(storage, args);
    }
    else if (args[0] == "rotate")
    {
      valid = zhuravleva::rotateShape(storage, args);
    }
    else if (args[0] == "set")
    {
      valid = zhuravleva::addSet(storage, args);
    }
    else if (args[0] == "show")
    {
      valid = zhuravleva::showShapes(storage, args, std::cout);
    }
    else if (args[0] == "showset")
    {
      valid = zhuravleva::showSets(storage, args, std::cout);
    }
    else if (args[0] == "frame")
    {
      valid = zhuravleva::showFrame(storage, args, std::cout);
    }
    else if (args[0] == "frameset")
    {
      valid = zhuravleva::showSetFrame(storage, args, std::cout);
    }
    else
    {
      std::cout << "<UNKNOWN COMMAND>\n";
      continue;
    }
    if (!valid)
    {
      std::cout << "<INVALID COMMAND>\n";
    }
  }

  return 0;
}
