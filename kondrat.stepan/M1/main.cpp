#include <iostream>
#include <vector>
#include <cstddef>
#include <stdexcept>
#include <string>

namespace kondrat
{
  struct Ellipse
  {
    int horizontalRadius;
    int verticalRadius;
    int x;
    int y;
  };

  struct Rectangle
  {
    double left;
    double right;
    double top;
    double bottom;
  };

  long long parseArgument(const char * argument);
  Rectangle findRectangle(const std::vector< Ellipse > & ellipses);
}

int main(int argc, char ** argv)
{
  if (argc != 3 && argc != 4)
  {
    std::cerr << "Usage: ./lab threads tries [seed]\n";
    return 1;
  }

  long long threads = 0;
  long long tries = 0;
  long long seed = 0;

  try
  {
    threads = kondrat::parseArgument(argv[1]);
    tries = kondrat::parseArgument(argv[2]);

    if (argc == 4)
    {
      seed = kondrat::parseArgument(argv[3]);
    }
  }
  catch (const std::invalid_argument & error)
  {
    std::cerr << error.what() << '\n';
    return 1;
  }
  catch (const std::out_of_range & error)
  {
    std::cerr << error.what() << '\n';
    return 1;
  }

  if (threads < 0 || tries <= 0 || seed < 0)
  {
    std::cerr << "Threads and seed must be nonnegative, tries must be positive\n";
    return 1;
  }

  std::vector< kondrat::Ellipse > ellipses;
  int a = 0;
  int b = 0;
  int cx = 0;
  int cy = 0;

  while (std::cin >> a >> b >> cx >> cy)
  {
    const int verticalRadius = b == 0 ? a : b;
    ellipses.push_back({a, verticalRadius, cx, cy});
  }

  if (std::cin.bad() || !std::cin.eof())
  {
    std::cerr << "Failed to parse figure\n";
    return 1;
  }

  if (ellipses.empty())
  {
    std::cout << "0 0\n";
    return 0;
  }
}

long long kondrat::parseArgument(const char * argument)
{
  size_t position = 0;
  const long long value = std::stoll(argument, &position);

  if (argument[position] != '\0')
  {
    throw std::invalid_argument("Invalid argument");
  }

  return value;
}

kondrat::Rectangle kondrat::findRectangle(const std::vector< Ellipse > & ellipses)
{
  const Ellipse first = ellipses.front();
  const double firstX = first.x;
  const double firstY = first.y;

  Rectangle rectangle =
  {
    firstX - first.horizontalRadius,
    firstX + first.horizontalRadius,
    firstY - first.verticalRadius,
    firstY - first.verticalRadius
  };

  for (const Ellipse & ellipse : ellipses)
  {
    const double x = ellipse.x;
    const double y = ellipse.y;

    rectangle.left = std::min(rectangle.left, x - ellipse.horizontalRadius);
    rectangle.right = std::max(rectangle.right, x + ellipse.horizontalRadius);
    rectangle.top = std::max(rectangle.top, y + ellipse.verticalRadius);
    rectangle.bottom = std::min(rectangle.bottom, y - ellipse.verticalRadius);
  }

  return rectangle;
}
