#include "path/pathpoint.h"
#include "path/path.h"
#include "path/pathvector.h"
#include "objects/pathobject.h"
#include "scene/disjointpathpointsetforest.h"
#include "scene/scene.h"

namespace omm
{

PathPoint::PathPoint(const Point& geometry, Path& path)
  : m_geometry(geometry)
  , m_path(path)
{
}

PathPoint::PathPoint(Path& path)
  : m_path(path)
{
}

std::set<PathPoint*> PathPoint::joined_points() const
{
  return path_vector()->joined_points().get(this);
}

void PathPoint::join(std::set<PathPoint*> buddies)
{
  buddies.insert(this);
  path_vector()->joined_points().insert(buddies);
}

void PathPoint::disjoin()
{
  path_vector()->joined_points().remove({this});
}

PathVector* PathPoint::path_vector() const
{
  return m_path.path_vector();
}

Point PathPoint::compute_joined_point_geometry(PathPoint& joined) const
{
  const auto controller_t = path_vector()->path_object()->global_transformation(Space::Scene);
  const auto agent_t = joined.path_vector()->path_object()->global_transformation(Space::Scene);
  const auto t = agent_t.inverted().apply(controller_t);
  auto geometry = joined.geometry();
  geometry.set_position(t.apply(this->geometry()).position());

  // TODO handle tangents

  return geometry;
}

std::size_t PathPoint::index() const
{
  assert(path_vector() != nullptr);
  std::size_t offset = 0;
  for (auto* path : path_vector()->paths()) {
    if (path->contains(*this)) {
      return offset + path->find(*this);
    } else {
      offset += path->size();
    }
  }
  throw std::runtime_error("Point is not part of a path.");
}

void PathPoint::set_geometry(const Point& point)
{
  m_geometry = point;
}

const Point& PathPoint::geometry() const
{
  return m_geometry;
}

PathPoint PathPoint::copy(Path& path) const
{
  return PathPoint(m_geometry, path);
}

Path& PathPoint::path() const
{
  return m_path;
}

bool PathPoint::is_selected() const
{
  return m_is_selected;
}

void PathPoint::set_selected(bool selected, bool update_buddies)
{
  m_is_selected = selected;
  if (update_buddies) {
    for (auto* buddy : joined_points()) {
      buddy->set_selected(selected, false);
    }
  }
}

}  // namespace omm