#include <iostream>
#include <cstddef>
#include <vector>

namespace petrov
{
  struct p_t {
    int x_;
    int y_;
  };

  struct Ellipse {
    p_t center_;
    int radius1_;
    int radius2_;

    Ellipse(int r1, int r2, int x, int y):
      center_{x, y},
      radius1_(r1),
      radius2_(r2 > 0 ? r2 : r1)
    {}
  };

  struct UniBox {
    int maxX, maxY, minX, minY;
  };

  UniBox calcBigBox(std::vector< petrov::Ellipse > shapes)
  {
    UniBox res{
      shapes[0].center_.x_ + shapes[0].radius2_,
      shapes[0].center_.y_ + shapes[0].radius2_,
      shapes[0].center_.x_ - shapes[0].radius2_,
      shapes[0].center_.y_ + shapes[0].radius2_};
    for (int i = 1; i < shapes.size(); ++i)
    {
      res.maxX = std::max(res.maxX, shapes[i].center_.x_ + shapes[i].radius2_);
      res.maxY = std::max(res.maxX, shapes[i].center_.y_ + shapes[i].radius2_);
      res.minX = std::min(res.minX, shapes[i].center_.x_ - shapes[i].radius2_);
      res.minY = std::min(res.minY, shapes[i].center_.y_ - shapes[i].radius2_);
    }

    return res;
  }

  bool isINside(const Ellipse& shape, const p_t& point)
  {
    const double dx = (point.x_ - shape.center_.x_) / shape.radius2_;
    const double dy = (point.y_ - shape.center_.y_) / shape.radius1_;

    return dx * dx + dy * dy <= 1;
  }
}

int main(int argc, char const *argv[])
{
  if (argc < 3 || argc > 4)
  {
    std::cerr << "invalid arguments count\n";
    return 1;
  }

  int threads, tries, seed = 0;
  try
  {
    threads = std::stoll(argv[1]);
    tries = std::stoll(argv[2]);

    if (argc == 4)
    {
      seed = std::stoll(argv[3]);
    }
    else
    {
      seed = 0;
    }

    if (threads < 1 || tries < 1 || seed < 0)
    {
      std::cerr << "Arguments invalid\n";
    }
  }
  catch(...)
  {
    std::cerr << "Invalid arguments" << '\n';
    return 1;
  }

  std::vector< petrov::Ellipse > shapes;

  int r1, r2, x, y = 0;
  while (std::cin >> r1 >> r2 >> x >> y)
  {
    if (r1 <= 0 || r2 < 0)
    {
      std::cerr << "Radius invalid\n";
      return 1;
    }

    shapes.push_back(petrov::Ellipse(r1, r2, x, y));
  }
  
  if (!std::cin.eof())
  {
    std::cerr << "Failed to parse figure\n";
    return 1;
  }

  if (shapes.empty())
  {
    std::cerr << "No figures provided\n";
    return 1;
  }

  
  return 0;
}

