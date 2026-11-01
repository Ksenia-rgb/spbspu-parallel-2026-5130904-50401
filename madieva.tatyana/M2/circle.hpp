#ifndef CIRCLE_HPP
#define CIRCLE_HPP

namespace madieva
{
  class Circle
  {
  public:
    Circle(long long r, long long cx, long long cy);
    long long getRadius() const;
    long long getX() const;
    long long getY() const;

  private:
    long long r_ = 0;
    long long x_ = 0;
    long long y_ = 0;
  };
}

#endif
