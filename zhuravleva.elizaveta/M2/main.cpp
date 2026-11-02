#include "commands.hpp"
#include "parse.hpp"
#include "process.hpp"
#include <iostream>
#include <string>
#include <vector>

int main()
{
  zhuravleva::Storage storage;
  zhuravleva::ProcessStorage process_storage;
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
    else if (args[0] == "spawn")
    {
      int seed = 0;
      valid = args.size() == 3 && zhuravleva::parseInt(args[2], seed) && seed >= 0;
      if (valid)
      {
        valid = zhuravleva::spawnProcess(process_storage, args[1], static_cast< unsigned int >(seed));
      }
    }
    else if (args[0] == "kill")
    {
      valid = args.size() == 2 && zhuravleva::killProcess(process_storage, args[1]);
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
  zhuravleva::shutdownProcesses(process_storage);
  return 0;
}
