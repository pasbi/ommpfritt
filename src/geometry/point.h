#pragma once

#include "geometry/vec2.h"
#include <Qt>
#include "geometry/polarcoordinates.h"

namespace omm
{

class Point
{
public:
  explicit Point( const Vec2f& position,
                  const PolarCoordinates& left_tangent,
                  const PolarCoordinates& right_tangent );
  explicit Point( const Vec2f& position,
                  const double rotation,
                  const double tangent_length = 1.0 );
  explicit Point( const Vec2f& position );
  Point();
  Vec2f position;
  Vec2f left_position() const;
  Vec2f right_position() const;
  double rotation() const;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("Point", "Point");
  bool is_selected = false;
  void swap(Point& other);

  PolarCoordinates left_tangent;
  PolarCoordinates right_tangent;

  Point smoothed(Vec2f &left_neighbor, const Vec2f &right_neighbor) const;
  Point nibbed() const;
  Point rotated(const double rad) const;

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
  Point flattened(const double t) const;

  bool operator==(const Point& point) const;
  bool operator!=(const Point& point) const;
  bool operator<(const Point& point) const;

  Point offset(double t, const Point *left_neighbor, const Point *right_neighbor) const;

  /**
   * @brief When a tangent is at `old_pos` and it is mirror-coupled with its sibling which moves
   *  from `old_other_pos` to `new_other_pos`, then this tangent moves to the return value of
   *  this function.
   * @param old_pos position of this tangent (relative to its Point)
   * @param old_other_pos old position of the other tangent (relative to its Point)
   * @param new_other_pos new position of the other tangent (relative to its Point)
   * @return [description]
   */
  static PolarCoordinates mirror_tangent( const PolarCoordinates& old_pos,
                                          const PolarCoordinates& old_other_pos,
                                          const PolarCoordinates& new_other_pos );

private:
  double get_direction(const Point* left_neighbor, const Point* right_neighbor) const;

public:
  static std::vector<Point> offset(const double t,
                                   const std::vector<Point>& points, const bool is_closed);

};

constexpr PolarCoordinates to_polar(Vec2f cartesian);
constexpr Vec2f to_cartesian(const PolarCoordinates& polar);

std::ostream& operator<<(std::ostream& ostream, const PolarCoordinates& pc);
std::ostream& operator<<(std::ostream& ostream, const Point& pc);
std::ostream& operator<<(std::ostream& ostream, const Point* pc);

}  // namespace omm
