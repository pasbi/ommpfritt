#pragma once

#include "geometry/rectangle.h"
#include <set>

class QRectF;

namespace omm
{
class Scene;
class BoundingBox : public Rectangle
{
public:
  using Rectangle::Rectangle;
  explicit BoundingBox(const std::set<Vec2f>& points);
  explicit BoundingBox(const std::set<double>& xs, const std::set<double>& ys);
  explicit BoundingBox(const std::set<Point>& points);
  explicit BoundingBox(const std::set<BoundingBox>& bbs);

  [[nodiscard]] bool contains(const BoundingBox& other) const;
  using Rectangle::contains;

  BoundingBox& operator|=(const BoundingBox& other);
  BoundingBox& operator|=(const Vec2f& point);

  static BoundingBox around_selected_objects(const Scene& scene);
  QString to_string() const;
};

BoundingBox operator|(const BoundingBox& a, const BoundingBox& b);
BoundingBox operator|(const BoundingBox& a, const Vec2f& b);

/**
 * @brief operator < There is no mathematical meaning behind this operator.
 *  It is only provided to enable BoundingBox as template-parameter in std::set.
 * @param a
 * @param b
 * @return returns whether @code a is smaller than @b
 */
bool operator<(const BoundingBox& a, const BoundingBox& b);
}  // namespace omm
