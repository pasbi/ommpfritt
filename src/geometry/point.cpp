#include "geometry/point.h"
#include <cmath>
#include <ostream>

namespace omm
{

Point::Point(const Vec2f& position,
             const PolarCoordinates& left_tangent, const PolarCoordinates& right_tangent)
  : position(position), left_tangent(left_tangent), right_tangent(right_tangent)
{
}

Point::Point(const Vec2f& position, const double rotation, const double tangent_length)
  : Point( position, PolarCoordinates(rotation, tangent_length),
                     PolarCoordinates(M_PI + rotation, tangent_length) )
{
}

Point::Point(const Vec2f& position) : Point(position, 0.0, 0.0) { }
Point::Point() : Point(Vec2f::o()) { }

Vec2f Point::left_position() const { return position + left_tangent.to_cartesian(); }
Vec2f Point::right_position() const { return position + right_tangent.to_cartesian(); }

void Point::swap(Point& other)
{
  other.position.swap(position);
  other.left_tangent.swap(left_tangent);
  other.right_tangent.swap(right_tangent);
}

Point Point::smoothed(Vec2f& left, const Vec2f& right) const
{
  auto copy = *this;
  const Vec2f d = left - right;
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
    ostream << "Point[[" << pc.position.x << ", " << pc.position.y << "], " << pc.left_tangent
            << ", " << pc.right_tangent << "]";
  } else {
    ostream << "[" << pc.position.x << ", " << pc.position.y << "]";
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
  return position == point.position
    && left_tangent == point.left_tangent
    && right_tangent == point.right_tangent;
}

bool Point::operator!=(const Point& point) const { return !(*this == point); }

}  // namespace omm
