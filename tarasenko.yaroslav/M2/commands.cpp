#include "commands.hpp"
#include <istream>
#include <cstring>
#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include "data.hpp"
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
    Ellipse* const ellipses = allocateSharedCircles(memory_name.c_str(), count);
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
    out << '(' << rectangle.left_bottom.x << ' ' << rectangle.left_bottom.y << ") (" << rectangle.right_top.x << ' '
        << rectangle.right_top.y << ")\n";
  }

  void frameSet(Data& data, std::istream& in, std::ostream& out)
  {
    std::string name;
    in >> name;
    const CircleSet& set = data.sets.at(name);
    const Rectangle rectangle = findFrame(set.ellipses, set.count);
    out << '(' << rectangle.left_bottom.x << ' ' << rectangle.left_bottom.y << ") (" << rectangle.right_top.x << ' '
        << rectangle.right_top.y << ")\n";
  }

  void spawn(Data& data, std::istream& in, std::ostream&)
  {
    std::string name;
    unsigned long seed = 0;
    in >> name >> seed;
    const std::string memory_name = "/worker_" + std::to_string(data.processes.size());
    WorkerState* const state
        = static_cast< WorkerState* >(allocateSharedMemory(memory_name.c_str(), sizeof(WorkerState)));
    *state = {0, false};
    const pid_t pid = fork();
    if (pid == -1) {
      freeSharedMemory(memory_name.c_str(), state, sizeof(WorkerState));
      throw std::runtime_error("Cannot create process");
    }
    if (pid == 0) {
      try {
        runWorker(seed, memory_name.c_str(), state);
      } catch (const std::exception&) {
        _exit(1);
      }
      _exit(0);
    }
    data.processes.emplace(name, Worker{pid, memory_name, state});
  }

  void areaOn(Data& data, std::istream& in, std::ostream&)
  {
    std::string process_name;
    std::string calculation_name;
    std::string set_name;
    long threads = 0;
    long tries = 0;
    in >> process_name >> calculation_name >> set_name >> threads >> tries;
    const Worker& worker = data.processes.at(process_name);
    const CircleSet& set = data.sets.at(set_name);
    const std::string memory_name = worker.memory_name + '_' + std::to_string(worker.state->submitted);
    Calculation* const calculation
        = static_cast< Calculation* >(allocateSharedMemory(memory_name.c_str(), sizeof(Calculation)));
    *calculation = {};
    std::strcpy(calculation->set_memory_name, set.memory_name.c_str());
    calculation->count = set.count;
    calculation->threads = threads == 0 ? 1 : threads;
    calculation->tries = tries;
    calculation->state = CalculationState::ready;
    data.calculations.emplace(calculation_name, CalculationInfo{process_name, memory_name, calculation});
    ++worker.state->submitted;
  }

  void status(Data& data, std::istream& in, std::ostream& out)
  {
    std::string name;
    in >> name;
    const CalculationInfo& info = data.calculations.at(name);
    const Calculation& calculation = *info.calculation;
    out << info.process_name << ' ';
    if (calculation.state == CalculationState::done) {
      const auto flags = out.flags();
      const auto precision = out.precision();
      out << std::fixed << std::setprecision(3) << calculation.milliseconds << ' ' << calculation.area << '\n';
      out.flags(flags);
      out.precision(precision);
    } else {
      out << "<IN PROGRESS>\n";
    }
  }

  void wait(Data&, std::istream&, std::ostream&)
  {}
}
