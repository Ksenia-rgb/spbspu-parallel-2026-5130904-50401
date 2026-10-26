#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <limits>
#include <cstdlib>
#include "commands.hpp"
#include "structures.hpp"
#include "worker.hpp"

int main(int argc, char** argv)
{
  (void)argc;
  const char* cmd_fd_env = std::getenv("CMD_RD");
  if (cmd_fd_env != nullptr)
  {
    int cmd_rd = std::stoi(cmd_fd_env);
    int res_wr = std::stoi(std::getenv("RES_WR"));
    unsigned int seed = std::stoi(std::getenv("SEED"));
    return vasyakin::worker(cmd_rd, res_wr, seed);
  }

  vasyakin::State state;
  state.exe_path = argv[0];

  using cmd_t = void (*)(std::istream&, std::ostream&, vasyakin::State&);

  std::unordered_map< std::string, cmd_t > cmds;

  cmds["circle"] = vasyakin::cmd_circle;
  cmds["set"] = vasyakin::cmd_set;
  cmds["show"] = vasyakin::cmd_show;
  cmds["showset"] = vasyakin::cmd_showset;
  cmds["frame"] = vasyakin::cmd_frame;
  cmds["frameset"] = vasyakin::cmd_frameset;
  cmds["spawn"] = vasyakin::cmd_spawn;
  cmds["area-on"] = vasyakin::cmd_areaon;
  cmds["status"] = vasyakin::cmd_status;
  cmds["wait"] = vasyakin::cmd_wait;

  std::string cmd;
  while (std::cin >> cmd)
  {
    try
    {
      cmds.at(cmd)(std::cin, std::cout, state);
    }
    catch (std::out_of_range& e)
    {
      std::cout << "<UNKNOWN COMMAND>" << '\n';
      std::cin.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
    }
    catch (std::invalid_argument& e)
    {
      std::cout << "<INVALID COMMAND>" << '\n';
      std::cin.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
    }
    catch (std::runtime_error& e)
    {
      std::cout << e.what() << '\n';
      std::cin.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
    }
  }

  return 0;
}
