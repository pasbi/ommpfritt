#include "geometry/polygon.h"
#include "geometry/triangle.h"
#include <algorithm>
#include <numeric>
#include "common.h"
#include "triangulation.h"

namespace
{

std::vector<omm::Vec2f> ensure_closed(std::vector<omm::Vec2f> points)
{
  if ((points.front() - points.back()).euclidean_norm() > 0.0001) {
    points.push_back(points.front());
  }
  return points;
}

}

namespace omm
{

Polygon::Polygon(const std::vector<Vec2f>& points)
  : m_points(ensure_closed(points))
{
  assert(m_points.size() > 3);  // at least a closed triangle
}

std::vector<Triangle> Polygon::triangulation() const
{
  triangulate();
  return m_triangles;
}

double Polygon::area() const
{
  triangulate();
  return m_area;
}

void Polygon::triangulate() const
{
  if (std::isnan(m_area)) {
    m_triangles = triangulate_delauney(m_points);
    m_triangle_areas = ::transform<double>(m_triangles, [](const Triangle& t) {
      return t.area();
    });
    m_area = std::accumulate(m_triangle_areas.begin(), m_triangle_areas.end(), 0.0);
    std::partial_sum( m_triangle_areas.begin(),
                      m_triangle_areas.end(),
                      std::back_inserter(m_triangle_areas_acc) );
  }
}

}  // namespace omm
