#include "geometry/triangle.h"

namespace omm
{

Triangle::Triangle(const std::array<Vec2f, 3>& points) : points(points) {}

double Triangle::area() const
{
  const auto& a = points[0];
  const auto& b = points[1];
  const auto& c = points[2];
  return 0.5 * (a.x-c.x)*(b.y-a.y) - (a.x-b.x)*(c.y-a.y);
}

}  // namespace omm
