#pragma once

#include "geometry/vec2.h"
#include <Qt>

namespace omm
{

namespace serialization
{
class SerializerWorker;
class DeserializerWorker;
}  // namespace serialization

struct PolarCoordinates
{
  explicit PolarCoordinates(double argument, double magnitude);
  explicit PolarCoordinates(const Vec2f& cartesian);
  explicit PolarCoordinates();
  [[nodiscard]] Vec2f to_cartesian() const;
  friend void swap(PolarCoordinates& a, PolarCoordinates& b);
  double argument;
  double magnitude;
  bool operator==(const PolarCoordinates& point) const;
  [[nodiscard]] bool has_nan() const;
  [[nodiscard]] bool has_inf() const;

  /**
   * @brief operator < enables the use of PolarCoordinates in, e.g., std::set
   */
  bool operator<(const PolarCoordinates& other) const;
  bool operator!=(const PolarCoordinates& point) const;
  PolarCoordinates operator-() const;

  /**
   * @brief normalize_angle normalizes an angle.
   * @param rad the angle
   * @return an number between [-pi, pi)
   */
  static double normalize_angle(double rad);

  QString to_string() const;
  void serialize(serialization::SerializerWorker& worker) const;
  void deserialize(serialization::DeserializerWorker& worker);
};

}  // namespace omm
