#pragma once

#include "geometry/vec2.h"
#include <QPainterPath>

namespace omm
{

class Point;
QPointF to_qpoint(const Vec2f& point);
Vec2f get_scale(const Vec2f& pos, const Vec2f& delta, const Vec2f& direction);
QPainterPath to_path(const std::vector<Point>& points, bool is_closed);

}  // namespace omm
