#include "geometry/point.h"
#include <cmath>
#include <glog/logging.h>

namespace omm
{


OrientedPoint::OrientedPoint() : position(arma::vec2{0.0, 0.0}), rotation(0), is_valid(false) { }

OrientedPoint::OrientedPoint(const arma::vec2& pos, double rotation)
  : position(pos), rotation(rotation), is_valid(true) { }

Point::Point(const arma::vec2& position)
  : position(position), left_tangent(PolarCoordinates()), right_tangent(PolarCoordinates()) { }

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

Point::Point(const OrientedPoint& op, const double tangent_length)
  : Point(op.position)
{
  left_tangent.argument = op.rotation;
  left_tangent.magnitude = tangent_length;
  right_tangent.argument = M_PI + op.rotation;
  right_tangent.magnitude = tangent_length;
}

Point Point::smoothed(const Point& left_neighbor, const Point& right_neighbor) const
{
  auto copy = *this;
  const PolarCoordinates l_pc(left_neighbor.position - copy.position);
  const PolarCoordinates r_pc(right_neighbor.position - copy.position);
  const double theta = (l_pc.argument + r_pc.argument) / 2.0;
  const double mag = (l_pc.magnitude + r_pc.magnitude) / 6.0;

  copy.left_tangent = PolarCoordinates(theta + M_PI_2, mag);
  copy.right_tangent = PolarCoordinates(theta - M_PI_2, mag);

  // that's a quick hack. If right tangent is closer to left position
  // than left tangent, then swap them.
  // I'm sure there's a more elegant way.
  if ( arma::norm(copy.right_position() - left_neighbor.position)
     < arma::norm(copy.left_position() - left_neighbor.position)  )
  {
    copy.left_tangent.swap(copy.right_tangent);
  }
  return copy;
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
