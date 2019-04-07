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
  explicit BoundingBox(const std::vector<Vec2f>& points = { Vec2f::o() });
  explicit BoundingBox(const std::vector<double>& xs, const std::vector<double>& ys);

  BoundingBox merge(const BoundingBox& other) const;
  BoundingBox intersect(const BoundingBox& other) const;
  bool contains(const BoundingBox& other) const;
  using Rectangle::contains;
};

std::ostream& operator<<(std::ostream& ostream, const BoundingBox& bb);

}  // namespace omm
