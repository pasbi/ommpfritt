#pragma once

#include <armadillo>

namespace omm
{

struct PolarCoordinates
{
  explicit PolarCoordinates(const double argument, const double magnitude);
  explicit PolarCoordinates(const arma::vec2& cartesian);
  explicit PolarCoordinates();
  arma::vec2 to_cartesian() const;
  void swap(PolarCoordinates& other);
  double argument;
  double magnitude;
};

class Point
{
public:
  explicit Point(const arma::vec2& position);
  Point();
  arma::vec2 position;
  arma::vec2 left_position() const;
  arma::vec2 right_position() const;
  static constexpr auto TYPE = "Point";
  bool is_selected = false;
  void swap(Point& other);

  PolarCoordinates left_tangent;
  PolarCoordinates right_tangent;

};

constexpr PolarCoordinates to_polar(arma::vec2 cartesian);
constexpr arma::vec2 to_cartesian(const PolarCoordinates& polar);

}  // namespace omm
