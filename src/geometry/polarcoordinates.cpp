#include "geometry/polarcoordinates.h"

namespace omm
{

PolarCoordinates::PolarCoordinates(const double argument, const double magnitude)
  : argument(argument), magnitude(magnitude) { }

PolarCoordinates::PolarCoordinates(const Vec2f& cartesian)
  : PolarCoordinates(cartesian.arg(), cartesian.euclidean_norm())
{
}

PolarCoordinates::PolarCoordinates() : PolarCoordinates(0.0, 0.0) {}

Vec2f PolarCoordinates::to_cartesian() const
{
  return Vec2f { magnitude * std::cos(argument), magnitude * std::sin(argument) };
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

bool PolarCoordinates::operator<(const PolarCoordinates &other) const
{
  if (magnitude == other.magnitude) {
    return argument < other.argument;
  } else {
    return magnitude < other.magnitude;
  }
}

bool PolarCoordinates::operator!=(const PolarCoordinates& point) const
{
  return !(*this == point);
}

PolarCoordinates PolarCoordinates::operator-() const
{
  return PolarCoordinates(normalize_angle(argument + M_PI), magnitude);
}

double PolarCoordinates::normalize_angle(double rad)
{
  static constexpr auto pi2 = 2*M_PI;
  return fmod(fmod(rad + M_PI, pi2) + pi2, pi2) - M_PI;
}

}  // namespace om
