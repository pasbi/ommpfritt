#include "path/pathpoint.h"
#include "path/path.h"
#include "path/pathvector.h"
#include "objects/pathobject.h"
#include "scene/disjointpathpointsetforest.h"
#include "scene/scene.h"

namespace omm
{

PathPoint::PathPoint(const Point& geometry, PathVector* path_vector)
  : m_path_vector(path_vector)
  , m_geometry(geometry)
{
}

PathVector* PathPoint::path_vector() const
{
  return m_path_vector;
}

QString PathPoint::debug_id() const
{
  return QString{"%1"}.arg(index());
}

std::size_t PathPoint::index() const
{
  assert(path_vector() != nullptr);
  const auto points = path_vector()->points();
  return std::distance(points.begin(), std::find(points.begin(), points.end(), this));
}

void PathPoint::set_geometry(const Point& point)
{
  m_geometry = point;
}

const Point& PathPoint::geometry() const
{
  return m_geometry;
}

bool PathPoint::is_selected() const
{
  return m_is_selected;
}

void PathPoint::set_selected(bool selected)
{
  m_is_selected = selected;
}

}  // namespace omm
