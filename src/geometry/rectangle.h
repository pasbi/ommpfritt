#pragma once

#include <QRectF>
#include "geometry/vec2.h"
#include "geometry/point.h"

namespace omm
{

class Rectangle
{
public:
  explicit Rectangle(const QRectF& rect);
  explicit Rectangle(const Vec2f& top_left, const Vec2f& bottom_right);
  explicit Rectangle(const Vec2f& center, const double radius);

  operator QRectF() const;

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

  std::vector<Point> to_points() const;

private:
  Vec2f m_top_left;
  Vec2f m_bottom_right;

};

}  // namespace omm
