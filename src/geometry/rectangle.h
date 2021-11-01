#pragma once

#include "geometry/vec2.h"
#include "geometry/point.h"
#include <QRectF>

namespace omm
{

class Rectangle
{
public:
  explicit Rectangle();
  explicit Rectangle(const QRectF& rect);
  explicit Rectangle(const Vec2f& top_left, const Vec2f& bottom_right, bool empty = false);
  explicit Rectangle(const Vec2f& center, double radius);

  operator QRectF() const;

  [[nodiscard]] bool contains(const Vec2f& point) const;
  [[nodiscard]] bool is_empty() const;

  [[nodiscard]] Vec2f top_left() const;
  [[nodiscard]] Vec2f top_right() const;
  [[nodiscard]] Vec2f bottom_left() const;
  [[nodiscard]] Vec2f bottom_right() const;
  [[nodiscard]] double left() const;
  [[nodiscard]] double top() const;
  [[nodiscard]] double right() const;
  [[nodiscard]] double bottom() const;
  [[nodiscard]] double width() const;
  [[nodiscard]] double height() const;
  [[nodiscard]] omm::Vec2f size() const;

  [[nodiscard]] std::vector<Point> to_points() const;

private:
  Vec2f m_top_left;
  Vec2f m_bottom_right;
  bool m_is_empty;
};

bool operator==(const Rectangle& a, const Rectangle& b);

}  // namespace omm
