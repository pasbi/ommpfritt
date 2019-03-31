#include "geometry/point.h"
#include <cmath>
#include <glog/logging.h>

namespace omm
{

Point::Point(const arma::vec2& position,
             const PolarCoordinates& left_tangent, const PolarCoordinates& right_tangent)
  : position(position), left_tangent(left_tangent), right_tangent(right_tangent)
{
}

Point::Point(const arma::vec2& position, const double rotation, const double tangent_length)
  : Point( position, PolarCoordinates(rotation, tangent_length),
                     PolarCoordinates(M_PI + rotation, tangent_length) )
{
}

Point::Point(const arma::vec2& position) : Point(position, 0.0, 0.0) { }
Point::Point() : Point(arma::vec2 { 0.0, 0.0 }) { }

arma::vec2 Point::left_position() const { return position + left_tangent.to_cartesian(); }
arma::vec2 Point::right_position() const { return position + right_tangent.to_cartesian(); }

void Point::swap(Point& other)
{
  other.position.swap(position);
  other.left_tangent.swap(left_tangent);
  other.right_tangent.swap(right_tangent);
}

PolarCoordinates::PolarCoordinates(const double argument, const double magnitude)
  : argument(argument), magnitude(magnitude) { }

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

bool PolarCoordinates::operator==(const PolarCoordinates& point) const
{
  return argument == point.argument && magnitude == point.magnitude;
}

bool PolarCoordinates::operator!=(const PolarCoordinates& point) const
{
  return !(*this == point);
}

Point Point::smoothed(arma::vec2& left, const arma::vec2& right) const
{
  auto copy = *this;
  const arma::vec2 d = left - right;
  copy.right_tangent = PolarCoordinates(-d/6.0);
  copy.left_tangent = PolarCoordinates(d/6.0);
  return copy;
}

double Point::rotation() const
{
  return PolarCoordinates(left_tangent).argument;
}

Point Point::nibbed() const
{
  auto copy = *this;
  copy.left_tangent.magnitude = 0;
  copy.right_tangent.magnitude = 0;
  return copy;
}

std::ostream& operator<<(std::ostream& ostream, const PolarCoordinates& pc)
{
  ostream << "[phi=" << pc.argument << ", r=" << pc.magnitude << "]";
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const Point& pc)
{
  static constexpr bool verbose = false;
  if constexpr (verbose) {
    ostream << "Point[[" << pc.position(0) << ", " << pc.position(1) << "], " << pc.left_tangent
            << ", " << pc.right_tangent << "]";
  } else {
    ostream << "[" << pc.position(0) << ", " << pc.position(1) << "]";
  }
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const Point* pc)
{
  if (pc == nullptr) {
    ostream << "Point[nullptr]";
  }
  ostream << *pc;
  return ostream;
}

bool Point::operator==(const Point& point) const
{
  return arma::all(position == point.position)
    && left_tangent == point.left_tangent
    && right_tangent == point.right_tangent;
}

bool Point::operator!=(const Point& point) const { return !(*this == point); }

}  // namespace omm
