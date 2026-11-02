#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "types.hpp"
#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include <memory>
#include <unistd.h>
#include "shm.hpp"

namespace hvostov {
  struct process_t {
    pid_t pid = -1;
    int to_child = -1;
    int from_child = -1;
    std::shared_ptr< ShmGuard > shm;
    std::vector< std::string > pending;
    size_t seed = 0;
  };

  struct info_t {
    std::unordered_map< std::string, shape_t > db;
    std::unordered_map< std::string, std::vector< shape_t > > set;
    std::unordered_map< std::string, process_t > procs;
    std::unordered_map< std::string, calc_state_t > calc;
  };

  using cmd_t = std::function< void(std::istream&, std::ostream&, info_t&) >;

  void addCircle(std::istream& in, std::ostream& out, info_t& info);
  void addEllipse(std::istream& in, std::ostream& out, info_t& info);
  void rotate(std::istream& in, std::ostream& out, info_t& info);
  void show(std::istream& in, std::ostream& out, info_t& info);
  void set(std::istream& in, std::ostream& out, info_t& info);
  void showSet(std::istream& in, std::ostream& out, info_t& info);
  void getFrame(std::istream& in, std::ostream& out, info_t& info);
  void getFrameSet(std::istream& in, std::ostream& out, info_t& info);
  void spawnProcess(std::istream& in, std::ostream& out, info_t& info);
  void areaOn(std::istream& in, std::ostream& out, info_t& info);
  void status(std::istream& in, std::ostream& out, info_t& info);
  void wait(std::istream& in, std::ostream& out, info_t& info);
  void timeout(std::istream& in, std::ostream& out, info_t& info);
  void killProcess(std::istream& in, std::ostream& out, info_t& info);
}

#endif
