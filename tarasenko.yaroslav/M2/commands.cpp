#include "commands.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include "Data.hpp"
#include "worker.hpp"

namespace tarasenko {
  void circle(Data& data, std::istream& in, std::ostream&)
  {
    std::string name;
    Ellipse shape{};
    in >> name >> shape.rx >> shape.center.x >> shape.center.y;
    shape.ry = shape.rx;
    data.ellipses.emplace(name, shape);
  }

  void show(Data& data, std::istream& in, std::ostream& out)
  {
    std::string name;
    in >> name;
    const Ellipse& shape = data.ellipses.at(name);
    out << shape.rx << " (" << shape.center.x << ' ' << shape.center.y << ")\n";
  }

  void set(Data& data, std::istream& in, std::ostream&)
  {
    std::string name;
    size_t count = 0;
    in >> name >> count;
    const std::string memory_name = "/m2_" + std::to_string(data.sets.size());
    Ellipse* ellipses = allocateSharedCircles(memory_name.c_str(), count);
    for (size_t i = 0; i < count; ++i) {
      std::string ellipse_name;
      in >> ellipse_name;
      ellipses[i] = data.ellipses.at(ellipse_name);
    }
    data.sets.emplace(name, CircleSet{memory_name, ellipses, count});
  }

  void showSet(Data& data, std::istream& in, std::ostream& out)
  {
    std::string name;
    in >> name;
    const CircleSet& set = data.sets.at(name);
    for (size_t i = 0; i < set.count; ++i) {
      const Ellipse& shape = set.ellipses[i];
      out << shape.rx << " (" << shape.center.x << ' ' << shape.center.y << ")\n";
    }
  }

  void frame(Data& data, std::istream& in, std::ostream& out)
  {
    std::string name;
    in >> name;
    const Ellipse& shape = data.ellipses.at(name);
    const Rectangle rectangle = findFrame(&shape, 1);
    out << '(' << rectangle.left_bottom.x << ' ' << rectangle.left_bottom.y << ") ("
        << rectangle.right_top.x << ' ' << rectangle.right_top.y << ")\n";
  }

  void frameSet(Data& data, std::istream& in, std::ostream& out)
  {
    std::string name;
    in >> name;
    const CircleSet& set = data.sets.at(name);
    const Rectangle rectangle = findFrame(set.ellipses, set.count);
    out << '(' << rectangle.left_bottom.x << ' ' << rectangle.left_bottom.y << ") ("
        << rectangle.right_top.x << ' ' << rectangle.right_top.y << ")\n";
  }

  void spawn(Data& data, std::istream& in, std::ostream&)
  {
    std::string name;
    unsigned long seed = 0;
    in >> name >> seed;
    const pid_t pid = fork();
    if (pid == -1) {
      throw std::runtime_error("Cannot create process");
    }
    if (pid == 0) {
      runWorker(seed);
      _exit(0);
    }
    data.processes.emplace(name, pid);
  }

  void areaOn(Data&, std::istream&, std::ostream&)
  {}

  void status(Data&, std::istream&, std::ostream&)
  {}

  void wait(Data&, std::istream&, std::ostream&)
  {}
}
