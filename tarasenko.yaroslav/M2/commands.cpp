#include "commands.hpp"
#include <istream>
#include <ostream>
#include <string>
#include "Data.hpp"

namespace tarasenko {
  void circle(Data& context, std::istream& in, std::ostream&)
  {
    std::string name;
    Circle shape{};
    in >> name >> shape.radius >> shape.center.x >> shape.center.y;
    context.circles.emplace(name, shape);
  }

  void show(Data& context, std::istream& in, std::ostream& out)
  {
    std::string name;
    in >> name;
    const Circle& shape = context.circles.at(name);
    out << shape.radius << " (" << shape.center.x << ' ' << shape.center.y << ")\n";
  }

  void set(Data&, std::istream&, std::ostream&)
  {}

  void showSet(Data&, std::istream&, std::ostream&)
  {}

  void frame(Data&, std::istream&, std::ostream&)
  {}

  void frameSet(Data&, std::istream&, std::ostream&)
  {}

  void spawn(Data&, std::istream&, std::ostream&)
  {}

  void areaOn(Data&, std::istream&, std::ostream&)
  {}

  void status(Data&, std::istream&, std::ostream&)
  {}

  void wait(Data&, std::istream&, std::ostream&)
  {}
}
