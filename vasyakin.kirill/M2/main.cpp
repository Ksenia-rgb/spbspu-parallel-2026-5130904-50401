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

  std::string cmd;
  while (std::cin >> cmd)
  {
    try
    {
      cmds.at(cmd)(std::cin, std::cout, state);

      int next = std::cin.peek();
      while (next == ' ' || next == '\t' || next == '\r')
      {
        std::cin.get();
        next = std::cin.peek();
      }

      if (next != '\n' && next != EOF)
      {
        throw std::invalid_argument("trailing arguments");
      }
    }
    catch (std::out_of_range& e)
    {
      std::cout << "<UNKNOWN COMMAND>" << '\n';
      std::cin.clear();
      std::cin.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
    }
    catch (std::invalid_argument& e)
    {
      std::cout << "<INVALID COMMAND>" << '\n';
      std::cin.clear();
      std::cin.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
    }
    catch (std::runtime_error& e)
    {
      std::cout << e.what() << '\n';
      std::cin.clear();
      std::cin.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
    }
  }

  return 0;
}
