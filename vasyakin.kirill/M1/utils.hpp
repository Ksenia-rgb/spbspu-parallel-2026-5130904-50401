#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <string>
#include <utility>
#include <memory>

namespace vasyakin
{
  class Figure
  {
  public:
    virtual ~Figure() = default;

    virtual bool contains(double dx, double dy) const noexcept = 0;

    virtual double getMinX() const noexcept = 0;
    virtual double getMinY() const noexcept = 0;
    virtual double getMaxX() const noexcept = 0;
    virtual double getMaxY() const noexcept = 0;
  };

  class Circle: public Figure
  {
  public:
    Circle(double radius, double x, double y) noexcept;

    bool contains(double dx, double dy) const noexcept override;

    double getMinX() const noexcept override;
    double getMinY() const noexcept override;
    double getMaxX() const noexcept override;
    double getMaxY() const noexcept override;

  private:
    double radius_;
    double x_, y_;
  };

  class Ellipse: public Figure
  {
  public:
    Ellipse(double a, double b, double x, double y) noexcept;

    bool contains(double dx, double dy) const noexcept override;

    double getMinX() const noexcept override;
    double getMinY() const noexcept override;
    double getMaxX() const noexcept override;
    double getMaxY() const noexcept override;

  private:
    double a_, b_;
    double x_, y_;
  };

  struct rectangle_t
  {
    double min_x, min_y, max_x, max_y;
  };

  rectangle_t findRectangleRange(const std::vector< std::unique_ptr< vasyakin::Figure > >& circles);

  long long parseArgument(const char* arg, const std::string& param);

  std::pair< long long, long long > calc(long long tries, long long seed, double min_x, double max_x, double min_y,
      double max_y, const std::vector< std::unique_ptr< vasyakin::Figure > >& circles);

  std::pair< double, double > area(long long threads, long long tries, long long seed,
      const std::vector< std::unique_ptr< vasyakin::Figure > >& circles, rectangle_t rect);
}

#endif
