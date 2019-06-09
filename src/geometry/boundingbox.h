#pragma once

#include "geometry/boundingbox.h"
#include "geometry/vec2.h"
#include "geometry/rectangle.h"

class QRectF;

namespace omm
{

class BoundingBox : public Rectangle
{
public:
  using Rectangle::Rectangle;
  explicit BoundingBox(const std::vector<Vec2f>& points);
  explicit BoundingBox(const std::vector<double>& xs, const std::vector<double>& ys);
  explicit BoundingBox(const std::vector<Point>& points);

  bool contains(const BoundingBox& other) const;
  using Rectangle::contains;

  BoundingBox& operator |=(const BoundingBox& other);
  BoundingBox& operator |=(const Vec2f& point);
};

std::ostream& operator<<(std::ostream& ostream, const BoundingBox& bb);
BoundingBox operator|(const BoundingBox& a, const BoundingBox& b);
BoundingBox operator|(const BoundingBox& a, const Vec2f& b);

}  // namespace omm
