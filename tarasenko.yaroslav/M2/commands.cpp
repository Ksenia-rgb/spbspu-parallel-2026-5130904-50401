#include "commands.hpp"
#include <istream>
#include <ostream>
#include <string>
#include "Data.hpp"

namespace tarasenko {
  void circle(Data& data, std::istream& in, std::ostream&)
  {
    std::string name;
    Circle shape{};
    in >> name >> shape.radius >> shape.center.x >> shape.center.y;
    data.circles.emplace(name, shape);
  }

  void show(Data& data, std::istream& in, std::ostream& out)
  {
    std::string name;
    in >> name;
    const Circle& shape = data.circles.at(name);
    out << shape.radius << " (" << shape.center.x << ' ' << shape.center.y << ")\n";
  }

  void set(Data&, std::istream&, std::ostream&)
  {}

  void showSet(Data&, std::istream&, std::ostream&)
  {}

  void frame(Data& data, std::istream& in, std::ostream& out)
  {
    std::string name;
    in >> name;
    const Circle& shape = data.circles.at(name);
    const Rectangle rectangle = {{shape.center.x - shape.radius, shape.center.y - shape.radius},
                                 {shape.center.x + shape.radius, shape.center.y + shape.radius}};
    out << '(' << rectangle.left_bottom.x << ' ' << rectangle.left_bottom.y << ") ("
        << rectangle.right_top.x << ' ' << rectangle.right_top.y << ")\n";
  }

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
