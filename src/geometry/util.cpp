#include "geometry/util.h"
#include "geometry/point.h"
#include <cmath>

namespace omm
{

QPointF to_qpoint(const Vec2f& point)
{
  return QPointF(point[0], point[1]);
}

Vec2f get_scale(const Vec2f& pos, const Vec2f& delta, const Vec2f& direction)
{
  const auto factor = [](const double new_pos, const double delta, const double constraint) {
    const double old_pos = new_pos - delta;
    const double epsilon = 0.0001;
    if (std::abs(old_pos) < epsilon) {
      return 1.0;
    } else {
      double s = new_pos / old_pos;
      s = std::pow(s, std::abs(constraint));
      return s;
    }
  };
  const Vec2f d = direction / direction.euclidean_norm();
  return Vec2f(factor(pos.x, delta.x, d.x), factor(pos.y, delta.y, d.y));
}

QPainterPath to_path(const std::vector<Point>& points, bool is_closed)
{
  QPainterPath path;
  if (points.size() > 1) {
    path.moveTo(to_qpoint(points.front().position));

    for (size_t i = 1; i < points.size(); ++i)
    {
      path.cubicTo( to_qpoint(points.at(i-1).right_position()),
                    to_qpoint(points.at(i).left_position()),
                    to_qpoint(points.at(i).position)  );
    }

    if (is_closed && points.size() > 2) {
      path.cubicTo(to_qpoint(points.back().right_position()),
                   to_qpoint(points.front().left_position()),
                   to_qpoint(points.front().position) );
    }

  }
  return path;
}

}  // namespace omm
