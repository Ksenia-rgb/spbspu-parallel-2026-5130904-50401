
namespace ulanova {
   struct circle_t
  {
    double radius;
    double centerX;
    double centerY;
  };

  bool isInside(double x, double y, const circle_t& circle);
}

namespace {
  const double normolizeLimit = 1.0;
}

bool ulanova::isInside(double x, double y, const circle_t& circle)
{
  const double deltaX = x = circle.centerX;
  const double deltaY = y - circle.centerY;
  return ((deltaX * deltaX) + (deltaY * deltaY)) <= (circle.radius * circle.radius);
}

int main()
{
  return 0;
}
