#include "geometry/triangulation.h"
//#include <geos/triangulate/DelaunayTriangulationBuilder.h>
//#include <geos/geom/Polygon.h>
//#include <geos/geom/LinearRing.h>
//#include <geos/geom/CoordinateArraySequence.h>
//#include <geos/geom/PrecisionModel.h>
//#include <geos/geom/GeometryFactory.h>
#include <iostream>
#include "mainwindow/geos.h"
#include "common.h"
#include "logging.h"
#include "mainwindow/application.h"

namespace omm
{

//std::vector<Triangle> triangulate_cpp(const std::vector<Vec2f>& points)
//{
//  geos::geom::PrecisionModel pm(2.0, 0, 0);
//  auto factory = geos::geom::GeometryFactory::create(&pm, -1);
//  const auto to_coordinate = [](const Vec2f& v) { return geos::geom::Coordinate(v.x, v.y); };
//  const auto to_vec2f = [](const geos::geom::Coordinate& c) { return Vec2f(c.x, c.y); };
//  auto vector = ::transform<geos::geom::Coordinate>(points, to_coordinate);
//  auto sequence = std::make_unique<geos::geom::CoordinateArraySequence>(&vector);
//  auto shell = std::make_unique<geos::geom::LinearRing>(sequence.release(), factory.get());
//  auto polygon = factory->createPolygon(shell.release(), nullptr);

//  geos::triangulate::DelaunayTriangulationBuilder builder;
//  builder.setSites(*polygon);
//  const auto collection = builder.getTriangles(*factory);

//  std::vector<Triangle> triangles;
//  triangles.reserve(collection->getNumGeometries());
//  for (std::size_t i = 0; i < collection->getNumGeometries(); ++i) {
//    const auto* geometry = collection->getGeometryN(i);
//    auto sequence = std::unique_ptr<geos::geom::CoordinateSequence>(geometry->getCoordinates());
//    assert(sequence->size() == 3);

//    const Vec2f a = to_vec2f(sequence->getAt(0));
//    const Vec2f b = to_vec2f(sequence->getAt(1));
//    const Vec2f c = to_vec2f(sequence->getAt(2));
//    triangles.emplace_back(std::array{ a, b, c });
//  }

//  return triangles;
//}

std::vector<Triangle> triangulate_delauney(const std::vector<Vec2f>& points)
{
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
  const double tolerance = 0.0001;
  GEOSGeometry* triangulation = GEOSDelaunayTriangulation_r(gch, polygon, tolerance, false);
  assert(GEOSGeomTypeId_r(gch, triangulation) == GEOS_GEOMETRYCOLLECTION);
  const auto n_triangles = GEOSGetNumGeometries_r(gch, triangulation);
  assert(n_triangles >= 0);
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
    triangles.push_back(Triangle({ a, b, c  }));
  }
  return triangles;
}

}
