#include "geometry/triangulation.h"
#include <iostream>
#include "mainwindow/geos.h"
#include "common.h"
#include "logging.h"
#include "mainwindow/application.h"

namespace
{

bool has_perpendicular_edge(const omm::Triangle& triangle, const std::vector<omm::Vec2f>& points,
                            omm::Edge& new_diagonal, omm::Edge& old_diagonal)
{
  for (std::size_t i = 0; i < points.size(); ++i) {
    new_diagonal.a = points[i];
    new_diagonal.b = points[i==points.size()-1 ? 0 : i+1];
    if (triangle.crosses_edge(new_diagonal, old_diagonal)) {
      return true;
    }
  }
  return false;
}

}  // namespace

namespace omm
{

std::vector<Triangle> triangulate_delauney(const std::vector<Vec2f>& points)
{
  // Convert the points into a GEOS-polygon
  const auto& gch = Geos::instance().handle();
  GEOSCoordSequence* sequence = GEOSCoordSeq_create_r(gch, static_cast<uint>(points.size() + 1), 2);
  for (std::size_t i = 0; i < points.size(); ++i) {
    GEOSCoordSeq_setX_r(gch, sequence, static_cast<uint>(i), points[i].x);
    GEOSCoordSeq_setY_r(gch, sequence, static_cast<uint>(i), points[i].y);
  }
  GEOSCoordSeq_setX_r(gch, sequence, static_cast<uint>(points.size()), points[0].x);
  GEOSCoordSeq_setY_r(gch, sequence, static_cast<uint>(points.size()), points[0].y);
  assert(sequence != nullptr);

  GEOSGeometry* shell = GEOSGeom_createLinearRing_r(gch, sequence);
  assert(shell != nullptr);

  GEOSGeometry* polygon = GEOSGeom_createPolygon_r(gch, shell, nullptr, 0);
  assert(polygon != nullptr);

  // compute the delaunay-triangulation
  const double tolerance = 10e-10;
  GEOSGeometry* triangulation = GEOSDelaunayTriangulation_r(gch, polygon, tolerance, false);
  assert(GEOSGeomTypeId_r(gch, triangulation) == GEOS_GEOMETRYCOLLECTION);
  const auto n_triangles = GEOSGetNumGeometries_r(gch, triangulation);
  assert(n_triangles >= 0);

  // convert the GEOS-triangulation set into omm-classes
  std::vector<Triangle> triangles;
  triangles.reserve(static_cast<std::size_t>(n_triangles));
  for (int i = 0; i < n_triangles; ++i) {
    const auto* triangle = GEOSGetGeometryN_r(gch, triangulation, i);
    assert(GEOSGetNumCoordinates_r(gch, triangle) == 4);
    const auto* triangle_ext = GEOSGetExteriorRing_r(gch, triangle);
    const auto* sequence = GEOSGeom_getCoordSeq_r(gch, triangle_ext);
    Vec2f a, b, c;
    uint size;
    bool success = true;
    success &= static_cast<bool>(GEOSCoordSeq_getSize_r(gch, sequence, &size));
    success &= static_cast<bool>(GEOSCoordSeq_getX_r(gch, sequence, 0, &a.x));
    success &= static_cast<bool>(GEOSCoordSeq_getY_r(gch, sequence, 0, &a.y));
    success &= static_cast<bool>(GEOSCoordSeq_getX_r(gch, sequence, 1, &b.x));
    success &= static_cast<bool>(GEOSCoordSeq_getY_r(gch, sequence, 1, &b.y));
    success &= static_cast<bool>(GEOSCoordSeq_getX_r(gch, sequence, 2, &c.x));
    success &= static_cast<bool>(GEOSCoordSeq_getY_r(gch, sequence, 2, &c.y));
    assert(success);
    assert(size == 4);
    triangles.push_back(Triangle({ a, b, c }));
  }

  // Sometimes, a polygon edge E is not covered by an edge of the delauney-triangulation.
  // in that case, there is a quadrangle Q consisting out of two triangles p, q from the
  // triangulation. p and q have a common edge (a diagonal of Q) perpendicular to E.
  // p and q must be replaced with p' and q', forming the same quadrangle Q but using the other
  // diagonal of Q, which is E.
  for (std::size_t i = 0; i < triangles.size(); ++i) {
    if (!triangles[i].marked) {
      Edge old_diagonal, new_diagonal;
      if (has_perpendicular_edge(triangles[i], points, new_diagonal, old_diagonal)) {
        const auto pred =  [old_diagonal, t_i = triangles[i]](const omm::Triangle& t) {
          return t.has_edge(old_diagonal) && t != t_i;
        };
        const auto it = std::find_if(triangles.begin(), triangles.end(), pred);

        if (it != triangles.end()) {
          const std::size_t j = static_cast<std::size_t>(std::distance(triangles.begin(), it));
          triangles[i] = Triangle({ old_diagonal.a, new_diagonal.a, new_diagonal.b });
          triangles[j] = Triangle({ old_diagonal.b, new_diagonal.b, new_diagonal.a });
          triangles[i].marked = true;
          triangles[j].marked = true;
        } else {
          // May happen if the polygon intersects itself
        }
      }
    }
  }

  // remove all triangles that are not inside the polygon.
  const auto* prepared_polygon = GEOSPrepare_r(gch, polygon);
  triangles = ::filter_if(triangles, [prepared_polygon, gch](const Triangle& t) {
    auto* center_sequence = GEOSCoordSeq_create_r(gch, 1, 2);
    const Vec2f center = (t.points[0] + t.points[1] + t.points[2])/3.0;
    GEOSCoordSeq_setX_r(gch, center_sequence, 0, center.x);
    GEOSCoordSeq_setY_r(gch, center_sequence, 0, center.y);
    auto* center_point = GEOSGeom_createPoint_r(gch, center_sequence);
    const int contains = GEOSPreparedContains_r(gch, prepared_polygon, center_point);
    assert(contains == 1 || contains == 0);
    return contains == 1;
  });

  // done!
  return triangles;
}

}  // namespace omm
