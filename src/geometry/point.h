#pragma once

#include "geometry/polarcoordinates.h"
#include "geometry/vec2.h"
#include <Qt>

namespace omm
{

namespace serialization
{
class SerializerWorker;
class DeserializerWorker;
}  // namespace serialization

class Point
{
public:
  explicit Point(const Vec2f& position,
                 const PolarCoordinates& left_tangent,
                 const PolarCoordinates& right_tangent);
  explicit Point(const Vec2f& position, double rotation, double tangent_length = 1.0);
  explicit Point(const Vec2f& position);
  Point();
  [[nodiscard]] Vec2f position() const;
  void set_position(const Vec2f& position);
  [[nodiscard]] Vec2f left_position() const;
  void set_left_position(const Vec2f& position);
  [[nodiscard]] Vec2f right_position() const;
  void set_right_position(const Vec2f& position);
  [[nodiscard]] PolarCoordinates left_tangent() const;
  void set_left_tangent(const PolarCoordinates& vector);
  [[nodiscard]] PolarCoordinates right_tangent() const;
  void set_right_tangent(const PolarCoordinates& vector);
  [[nodiscard]] double rotation() const;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("Point", "Point");
  friend void swap(Point& a, Point& b);
  [[nodiscard]] bool has_nan() const;
  [[nodiscard]] bool has_inf() const;

  [[nodiscard]] Point rotated(double rad) const;
  [[nodiscard]] Point nibbed() const;
  [[nodiscard]] Point flipped() const;

  static constexpr auto POSITION_POINTER = "position";
  static constexpr auto LEFT_TANGENT_POINTER = "left";
  static constexpr auto RIGHT_TANGENT_POINTER = "right";

  void serialize(serialization::SerializerWorker& worker) const;
  void deserialize(serialization::DeserializerWorker& worker);

  /**
   * @brief flattened means adjust the tangents such that the angle between them approaches
   *  180 degree.
   * @param t control the amount of the effect.
   *  t = 0: returns a unmodified copy of the this.
   *  t = 1: return a copy where the tangents are spread by 180 degree.
   *  Values outside that range have not been tested, the result of such an operation is a surprise.
   *  The magnitude of tangents is not modified. The angle bisector is an invariant.
   * @return the flattened point.
   */
  [[nodiscard]] Point flattened(double t) const;

  bool operator==(const Point& point) const;
  bool operator!=(const Point& point) const;
  bool operator<(const Point& point) const;

  Point offset(double t, const Point* left_neighbor, const Point* right_neighbor) const;
  static std::vector<Point> offset(double t, const std::vector<Point>& points, bool is_closed);
  [[nodiscard]] QString to_string() const;

  /**
   * @brief When a tangent is at `old_pos` and it is mirror-coupled with its sibling which moves
   *  from `old_other_pos` to `new_other_pos`, then this tangent moves to the return value of
   *  this function.
   * @param old_pos position of this tangent (relative to its Point)
   * @param old_other_pos old position of the other tangent (relative to its Point)
   * @param new_other_pos new position of the other tangent (relative to its Point)
   * @return [description]
   */
  static PolarCoordinates mirror_tangent(const PolarCoordinates& old_pos,
                                         const PolarCoordinates& old_other_pos,
                                         const PolarCoordinates& new_other_pos);

private:
  double get_direction(const Point* left_neighbor, const Point* right_neighbor) const;
  Vec2f m_position;
  PolarCoordinates m_left_tangent;
  PolarCoordinates m_right_tangent;
};

constexpr PolarCoordinates to_polar(Vec2f cartesian);
constexpr Vec2f to_cartesian(const PolarCoordinates& polar);

bool fuzzy_eq(const Point& a, const Point& b);

}  // namespace omm
