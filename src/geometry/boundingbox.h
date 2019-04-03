#pragma once

#include "geometry/boundingbox.h"
#include "geometry/vec2.h"

namespace omm
{

class BoundingBox
{
public:
  BoundingBox(const std::vector<Vec2f>& points = { Vec2f::o() });

  BoundingBox merge(const BoundingBox& other) const;
  BoundingBox intersect(const BoundingBox& other) const;
  bool contains(const BoundingBox& other) const;
  bool contains(const Vec2f& point) const;
  bool is_empty() const;

  Vec2f top_left() const;
  Vec2f top_right() const;
  Vec2f bottom_left() const;
  Vec2f bottom_right() const;
  double left() const;
  double top() const;
  double right() const;
  double bottom() const;
  double width() const;
  double height() const;

private:
  Vec2f m_top_left;
  Vec2f m_bottom_right;
};

}  // namespace omm
