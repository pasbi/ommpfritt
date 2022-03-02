#include "geometry/polarcoordinates.h"
#include "serializers/serializerworker.h"
#include "serializers/deserializerworker.h"

namespace omm
{
PolarCoordinates::PolarCoordinates(const double argument, const double magnitude)
    : argument(argument), magnitude(magnitude)
{
}

PolarCoordinates::PolarCoordinates(const Vec2f& cartesian)
    : PolarCoordinates(cartesian.arg(), cartesian.euclidean_norm())
{
}

PolarCoordinates::PolarCoordinates() : PolarCoordinates(0.0, 0.0)
{
}

Vec2f PolarCoordinates::to_cartesian() const
{
  return Vec2f{magnitude * std::cos(argument), magnitude * std::sin(argument)};
}

void swap(PolarCoordinates& a, PolarCoordinates& b)
{
  std::swap(a.argument, b.argument);
  std::swap(a.magnitude, b.magnitude);
}

bool PolarCoordinates::operator==(const PolarCoordinates& point) const
{
  return argument == point.argument && magnitude == point.magnitude;
}

bool PolarCoordinates::has_nan() const
{
  return std::isnan(argument) || std::isnan(magnitude);
}

bool PolarCoordinates::has_inf() const
{
  return std::isinf(argument) || std::isinf(magnitude);
}

bool PolarCoordinates::operator<(const PolarCoordinates& other) const
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
  static constexpr auto pi2 = 2 * M_PI;
  return fmod(fmod(rad + M_PI, pi2) + pi2, pi2) - M_PI;
}

QString PolarCoordinates::to_string() const
{
  static constexpr auto rad_to_deg = 180.0 / M_PI;
  return QString{"[%1°, %2]"}.arg(argument * rad_to_deg).arg(magnitude);
}

void PolarCoordinates::serialize(serialization::SerializerWorker& worker) const
{
  worker.set_value(Vec2f{argument, magnitude});
}

void PolarCoordinates::deserialize(serialization::DeserializerWorker& worker)
{
  const auto v = worker.get<Vec2f>();
  argument = v.x;
  magnitude = v.y;
}

}  // namespace omm
