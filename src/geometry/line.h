#pragma once

#include "geometry/vec2.h"
#include <vector>

namespace omm
{

struct Line
{
  Vec2f a;
  Vec2f b;

  /**
   * @brief line_intersection computes the intersection of lines this and other.
   * @param other the other line
   * @return The intersection in line coordinates (wrt. this).
   */
  [[nodiscard]] double intersect(const Line& other) const noexcept;

  /**
   * @brief distance computes the distance between point p and this line.
   * @param p the point to test
   * @note This line is assumed to extend infintely beyond.
   * @note The distance is oriented, i.e., may be negative.
   */
  [[nodiscard]] double distance(const Vec2f& p) const noexcept;

  /**
   * @brief projects p onto this line  and returns the line coordinate of the projection.
   * @param p the point to project
   * @note The position of the projection can be computed with lerp(project(p)).
   */
  [[nodiscard]] double project(const Vec2f& p) const noexcept;

  /**
   * @brief lerp linearly interpolates between the start and the end point.
   * @param t
   * @return The start point if t=0 or the end point if t=1.0.
   */
  [[nodiscard]] Vec2f lerp(double t) const noexcept;

  [[nodiscard]] QString to_string() const;
  friend std::ostream& operator<<(std::ostream& os, const Line& line);
};

}  // namespace omm
