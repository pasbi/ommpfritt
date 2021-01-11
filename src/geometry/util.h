#pragma once

#include "logging.h"
#include "geometry/vec2.h"
#include <QPainterPath>

namespace omm
{

class Point;
QPointF to_qpoint(const Vec2f& point);
Vec2f get_scale(const Vec2f& pos, const Vec2f& delta, const Vec2f& direction);
QPainterPath to_path(const std::vector<Point>& points, bool is_closed);

template<typename Rect> double compute_aspect_ratio(Rect&& rect)
{
  return static_cast<double>(rect.width()) / static_cast<double>(rect.height());
};

/**
* @brief fit_rect_constant_aspect_ratio computes the largest rectangle of same aspect ratio as
*  @code target that fits inside bounding_rect.
* @param target the target rectangle
* @param bounding_rect the bounding rectangle
* @param align whether to align the rectangle top, left, bottom, right or central.
* @return the resized rectangle.
*/
template<typename Rect>
Rect resize_rect_keep_ar(const Rect& target, const Rect& bounding_rect, Qt::Alignment align)
{
  static const auto div = [](double a, double b) { return a / b; };

  static constexpr auto eps = 0.00001;
  Rect rect = target;
  static const auto area = [](auto&& rect) {
    return rect.width() * rect.height();
  };
  const double tar = std::abs(compute_aspect_ratio(target));
  const double bar = std::abs(compute_aspect_ratio(bounding_rect));
  if (area(target) < eps || area(bounding_rect) < eps) {
    return bounding_rect;
  } else if (target.height() < eps) {
    rect = Rect(0.0, 0.0, bounding_rect.width(), 0);
  } else if (tar <= bar) {
    const auto width = div(target.width(), target.height()) * bounding_rect.height();
    rect = Rect(0.0, 0.0, width, bounding_rect.height());
  } else if (tar >= bar) {
    const auto height = div(target.height(), target.width()) * bounding_rect.width();
    rect = Rect(0.0, 0.0, bounding_rect.width(), height);
  } else {
    Q_UNREACHABLE();
  }
  if (align & Qt::AlignLeft) {
    rect.moveLeft(bounding_rect.left());
  }
  if (align & Qt::AlignRight) {
    rect.moveRight(bounding_rect.right());
  }
  if (align & Qt::AlignHCenter) {
    rect.moveLeft(bounding_rect.center().x() - rect.width() / 2.0);
  }
  if (align & Qt::AlignTop) {
    rect.moveTop(bounding_rect.top());
  }
  if (align & Qt::AlignBottom) {
    rect.moveBottom(bounding_rect.bottom());
  }
  if (align & Qt::AlignVCenter) {
    rect.moveTop(bounding_rect.center().y() - rect.height() / 2.0);
  }
  return rect;
}

}  // namespace omm
