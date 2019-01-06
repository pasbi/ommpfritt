#include "geometry/point.h"
#include <cmath>

namespace omm
{

Point::Point(const arma::vec2& position)
  : position(position)
  , left_tangent(PolarCoordinates())
  , right_tangent(PolarCoordinates())
{
}

Point::Point() : Point(arma::vec2 { 0.0, 0.0 }) { }

arma::vec2 Point::left_position() const
{
  return position + left_tangent.to_cartesian();
}

arma::vec2 Point::right_position() const
{
  return position + right_tangent.to_cartesian();
}

void Point::swap(Point& other)
{
  other.position.swap(position);
  other.left_tangent.swap(left_tangent);
  other.right_tangent.swap(right_tangent);
}

PolarCoordinates::PolarCoordinates(const double argument, const double magnitude)
  : argument(argument), magnitude(magnitude)
{}

PolarCoordinates::PolarCoordinates(const arma::vec2& cartesian)
  : PolarCoordinates(std::atan2(cartesian[1], cartesian[0]), arma::norm(cartesian))
{
}

PolarCoordinates::PolarCoordinates() : PolarCoordinates(0.0, 0.0) {}

arma::vec2 PolarCoordinates::to_cartesian() const
{
  return arma::vec2 { magnitude * std::cos(argument), magnitude * std::sin(argument) };
}

void PolarCoordinates::swap(PolarCoordinates& other)
{
  std::swap(other.argument, argument);
  std::swap(other.magnitude, magnitude);
}

}  // namespace omm
