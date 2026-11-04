#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "shape_manager.hpp"
#include "process_manager.hpp"
#include "help_functions.hpp"
#include "commands.hpp"

int main()
{
  donkeev::ShapeManager shape_manager;
  donkeev::ProcessManager proc_manager;

  std::map< std::string, donkeev::Command > command_map = {{"circle", {donkeev::validateCircle, donkeev::executeCircle}},
      {"set", {donkeev::validateSet, donkeev::executeSet}},
      {"show", {donkeev::validateTwoTokens, donkeev::executeShow}},
      {"showset", {donkeev::validateTwoTokens, donkeev::executeShowSet}},
      {"frame", {donkeev::validateTwoTokens, donkeev::executeFrame}},
      {"frameset", {donkeev::validateTwoTokens, donkeev::executeFrameSet}},
      {"spawn", {donkeev::validateSpawn, donkeev::executeSpawn}},
      {"area-on", {donkeev::validateAreaOn, donkeev::executeAreaOn}},
      {"status", {donkeev::validateTwoTokens, donkeev::executeStatus}},
      {"wait", {donkeev::validateTwoTokens, donkeev::executeWait}}};

  std::string line;
  while (std::getline(std::cin, line))
  {
    if (line.empty())
    {
      continue;
    }

    auto tokens = donkeev::splitIntoTokens(line);
    if (tokens.empty())
    {
      continue;
    }

    const std::string& cmd_name = tokens[0];
    auto it = command_map.find(cmd_name);

    if (it == command_map.end())
    {
      std::cout << "<UNKNOWN COMMAND>\n";
      continue;
    }

    if (!it->second.validate(tokens))
    {
      std::cout << "<INVALID COMMAND>\n";
      continue;
    }

    try
    {
      it->second.execute(shape_manager, proc_manager, tokens);
    }
    catch (...)
    {
      std::cout << "<INVALID COMMAND>\n";
    }
  }

  return 0;
}
