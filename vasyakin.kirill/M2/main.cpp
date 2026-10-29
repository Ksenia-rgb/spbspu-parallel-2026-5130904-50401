#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <limits>
#include <cstdlib>
#include <string>
#include <cstdio>
#include "commands.hpp"
#include "structures.hpp"
#include "worker.hpp"
#include "buf.hpp"

int main(int argc, char** argv)
{
  (void)argc;
  const char* const cmd_fd_env = std::getenv("CMD_RD");
  if (cmd_fd_env != nullptr)
  {
    const int cmd_rd = std::stoi(cmd_fd_env);
    const int res_wr = std::stoi(std::getenv("RES_WR"));
    const unsigned int seed = std::stoi(std::getenv("SEED"));
    return vasyakin::worker(cmd_rd, res_wr, seed);
  }

  vasyakin::State state;
  state.exe_path = argv[0];

  using cmd_t = void (*)(std::istream&, std::ostream&, vasyakin::State&);

  std::unordered_map< std::string, cmd_t > cmds;

  cmds["circle"] = vasyakin::cmdCircle;
  cmds["set"] = vasyakin::cmdSet;
  cmds["show"] = vasyakin::cmdShow;
  cmds["showset"] = vasyakin::cmdShowset;
  cmds["frame"] = vasyakin::cmdFrame;
  cmds["frameset"] = vasyakin::cmdFrameset;
  cmds["spawn"] = vasyakin::cmdSpawn;
  cmds["area-on"] = vasyakin::cmdAreaon;
  cmds["status"] = vasyakin::cmdStatus;
  cmds["wait"] = vasyakin::cmdWait;

  std::string line;
  while (std::getline(std::cin, line))
  {
    if (!line.empty() && line.back() == '\r')
    {
      line.pop_back();
    }

    if (line.empty())
    {
      continue;
    }

    size_t pos = line.find_first_of(" \t");
    std::string cmd = (pos == std::string::npos) ? line : line.substr(0, pos);
    std::string args = (pos == std::string::npos) ? "" : line.substr(pos);

    try
    {
      vasyakin::string_buf sbuf(args);
      std::istream iss(&sbuf);

      cmds.at(cmd)(iss, std::cout, state);

      iss >> std::ws;
      if (iss.peek() != std::char_traits< char >::eof())
      {
        throw std::invalid_argument("trailing arguments");
      }
    }
    catch (std::out_of_range& e)
    {
      std::cout << "<UNKNOWN COMMAND>" << '\n';
    }
    catch (std::invalid_argument& e)
    {
      std::cout << "<INVALID COMMAND>" << '\n';
    }
    catch (std::runtime_error& e)
    {
      std::cout << e.what() << '\n';
    }
  }

  return 0;
}
