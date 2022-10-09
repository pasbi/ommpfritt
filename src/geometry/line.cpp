#include "geometry/line.h"

namespace omm
{

double Line::intersect(const Line& other) const noexcept
{
  // https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection#Given_two_points_on_each_line_segment
  const auto d = (a.x - other.a.x) * (other.a.y - other.b.y) - (a.y - other.a.y) * (other.a.x - other.b.x);
  const auto n = (a.x - b.x) * (other.a.y - other.b.y) - (a.y - b.y) * (other.a.x - other.b.x);
  return d / n;
}

Vec2f Line::lerp(const double t) const noexcept
{
  return a + t * (b - a);
}

QString Line::to_string() const
{
  return QString("Line({%1, %2}, {%3, %4})").arg(a.x).arg(a.y).arg(b.x).arg(b.y);
}

double Line::distance(const Vec2f& p) const noexcept
{
  // https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection#Given_two_points_on_each_line_segment
  const auto d = (b.x - a.x) * (a.y - p.y) - (a.x - p.x) * (b.y - a.y);
  const auto n = (a - b).euclidean_norm();
  return d / n;
}

double Line::project(const Vec2f& p) const noexcept
{
  const auto ab = b - a;
  const auto ap = p - a;
  return Vec2f::dot(ap, ab) / ab.euclidean_norm2();
}

}  // namespace omm
