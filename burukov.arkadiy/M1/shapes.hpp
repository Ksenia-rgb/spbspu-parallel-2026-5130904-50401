#ifndef SHAPES_HPP
#define SHAPES_HPP

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace burukov
{

  class Shape
  {
  public:
    virtual ~Shape() = default;

    virtual bool contains(double x, double y) const noexcept = 0;
    virtual double getMinX() const noexcept = 0;
    virtual double getMinY() const noexcept = 0;
    virtual double getMaxX() const noexcept = 0;
    virtual double getMaxY() const noexcept = 0;
  };

  class Circle: public Shape
  {
  public:
    Circle(double radius, double x, double y) noexcept;

    bool contains(double x, double y) const noexcept override;
    double getMinX() const noexcept override;
    double getMinY() const noexcept override;
    double getMaxX() const noexcept override;
    double getMaxY() const noexcept override;

  private:
    double radius_;
    double x_;
    double y_;
  };

  class Ellipse: public Shape
  {
  public:
    Ellipse(double a, double b, double x, double y) noexcept;

    bool contains(double x, double y) const noexcept override;
    double getMinX() const noexcept override;
    double getMinY() const noexcept override;
    double getMaxX() const noexcept override;
    double getMaxY() const noexcept override;

  private:
    double a_;
    double b_;
    double x_;
    double y_;
  };

  struct rectangle_t
  {
    double min_x;
    double min_y;
    double max_x;
    double max_y;
  };

  rectangle_t findRectangleRange(const std::vector< std::unique_ptr< Shape > >& shapes);

  long long parseArgument(const char* arg, const std::string& param);

  std::pair< long long, long long > calc(long long tries, long long seed, double min_x, double max_x, double min_y,
      double max_y, const std::vector< std::unique_ptr< Shape > >& shapes);

  std::pair< double, double > area(long long threads, long long tries, long long seed,
      const std::vector< std::unique_ptr< Shape > >& shapes, rectangle_t rect);

}

#endif
