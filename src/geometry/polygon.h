#pragma once

#include "geometry/vec2.h"
#include "geometry/triangle.h"
#include <vector>
#include <random>

namespace omm
{

class Polygon
{
public:
  Polygon(const std::vector<Vec2f>& points);
  std::vector<Triangle> triangulation() const;
  double area() const;
  template<typename Rng> Vec2f random_point(Rng&& rng) const
  {
    triangulate();
    std::uniform_real_distribution<> urd(0.0, m_area);
    const double t = urd(rng);

    const auto it = std::find_if( m_triangle_areas_acc.begin(),
                                  m_triangle_areas_acc.end(),
                                  [t](const double area) { return area >= t; } );
    const auto i = std::distance(m_triangle_areas_acc.begin(), it);
    return m_triangles[std::max(0, i-1)].random_point(rng);
  }

private:
  void triangulate() const;
  const std::vector<Vec2f> m_points;
  mutable std::vector<Triangle> m_triangles;
  mutable std::vector<double> m_triangle_areas;
  mutable std::vector<double> m_triangle_areas_acc;
  mutable double m_area = std::numeric_limits<double>::quiet_NaN();

};

}  // namespace omm
