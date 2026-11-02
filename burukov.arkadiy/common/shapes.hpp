#ifndef SHAPES_HPP
#define SHAPES_HPP

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace burukov
{
  struct rectangle_t
  {
    double min_x;
    double min_y;
    double max_x;
    double max_y;
  };

  struct shape_data_t
  {
    int kind;
    double a;
    double b;
    double x;
    double y;
    double angle;
  };

  class Shape
  {
  public:
    virtual ~Shape() = default;

    virtual bool contains(double x, double y) const noexcept = 0;
    virtual double getMinX() const noexcept = 0;
    virtual double getMinY() const noexcept = 0;
    virtual double getMaxX() const noexcept = 0;
    virtual double getMaxY() const noexcept = 0;
    virtual void rotate(double ksi) noexcept = 0;
    virtual void print() const = 0;
    virtual shape_data_t serialize() const noexcept = 0;
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
    void rotate(double ksi) noexcept override;
    void print() const override;
    shape_data_t serialize() const noexcept override;

  private:
    double radius_;
    double x_;
    double y_;
  };

  class Ellipse: public Shape
  {
  public:
    Ellipse(double a, double b, double x, double y, double angle = 0.0) noexcept;

    bool contains(double x, double y) const noexcept override;
    double getMinX() const noexcept override;
    double getMinY() const noexcept override;
    double getMaxX() const noexcept override;
    double getMaxY() const noexcept override;
    void rotate(double ksi) noexcept override;
    void print() const override;
    shape_data_t serialize() const noexcept override;

  private:
    double a_;
    double b_;
    double x_;
    double y_;
    double angle_;
  };

  rectangle_t findRectangleRange(const std::vector< std::unique_ptr< Shape > > & shapes);
  rectangle_t findDataRange(const std::vector< shape_data_t > & shapes);

  long long parseArgument(const char * arg, const std::string & param);

  std::pair< long long, long long > calc(long long tries, long long seed, double min_x, double max_x, double min_y,
      double max_y, const std::vector< std::unique_ptr< Shape > > & shapes);

  std::pair< double, double > area(long long threads, long long tries, long long seed,
      const std::vector< std::unique_ptr< Shape > > & shapes, rectangle_t rect);

  double calculateUnionArea(
      long long threads, long long tries, long long seed, const std::vector< shape_data_t > & shapes);
}

#endif
