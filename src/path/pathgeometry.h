#pragma once

#include "geometry/vec2.h"
#include "common.h"
#include <vector>

class QPainterPath;

namespace omm
{

class Point;
class PathGeometry
{
public:
  explicit PathGeometry(std::vector<Point> points);

  [[nodiscard]] static std::vector<Vec2f>
  compute_control_points(const Point& a, const Point& b, InterpolationMode interpolation = InterpolationMode::Bezier);
  [[nodiscard]] QPainterPath to_painter_path() const;
  [[nodiscard]] Point smoothen_point(std::size_t i) const;

private:
  std::vector<Point> m_points;
};

}  // namespace
