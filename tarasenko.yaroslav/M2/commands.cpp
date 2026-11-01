#include "commands.hpp"
#include <algorithm>
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

  void set(Data& data, std::istream& in, std::ostream&)
  {
    std::string name;
    size_t count = 0;
    in >> name >> count;
    const std::string memory_name = "/m2_" + std::to_string(data.sets.size());
    Circle* circles = allocateSharedCircles(memory_name.c_str(), count);
    for (size_t i = 0; i < count; ++i) {
      std::string circle_name;
      in >> circle_name;
      circles[i] = data.circles.at(circle_name);
    }
    data.sets.emplace(name, CircleSet{memory_name, circles, count});
  }

  void showSet(Data& data, std::istream& in, std::ostream& out)
  {
    std::string name;
    in >> name;
    const CircleSet& set = data.sets.at(name);
    for (size_t i = 0; i < set.count; ++i) {
      const Circle& shape = set.circles[i];
      out << shape.radius << " (" << shape.center.x << ' ' << shape.center.y << ")\n";
    }
  }

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

  void frameSet(Data& data, std::istream& in, std::ostream& out)
  {
    std::string name;
    in >> name;
    const CircleSet& set = data.sets.at(name);
    const Circle& first = set.circles[0];
    Rectangle rectangle = {{first.center.x - first.radius, first.center.y - first.radius},
                           {first.center.x + first.radius, first.center.y + first.radius}};
    for (size_t i = 1; i < set.count; ++i) {
      const Circle& shape = set.circles[i];
      rectangle.left_bottom.x = std::min(rectangle.left_bottom.x, shape.center.x - shape.radius);
      rectangle.left_bottom.y = std::min(rectangle.left_bottom.y, shape.center.y - shape.radius);
      rectangle.right_top.x = std::max(rectangle.right_top.x, shape.center.x + shape.radius);
      rectangle.right_top.y = std::max(rectangle.right_top.y, shape.center.y + shape.radius);
    }
    out << '(' << rectangle.left_bottom.x << ' ' << rectangle.left_bottom.y << ") ("
        << rectangle.right_top.x << ' ' << rectangle.right_top.y << ")\n";
  }

  void spawn(Data&, std::istream&, std::ostream&)
  {}

  void areaOn(Data&, std::istream&, std::ostream&)
  {}

  void status(Data&, std::istream&, std::ostream&)
  {}

  void wait(Data&, std::istream&, std::ostream&)
  {}
}
