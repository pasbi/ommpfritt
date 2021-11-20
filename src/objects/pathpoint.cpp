#include "objects/pathpoint.h"
#include "objects/segment.h"
#include "objects/path.h"
#include "scene/disjointpathpointsetforest.h"
#include "scene/scene.h"

namespace omm
{

PathPoint::PathPoint(const Point& geometry, Segment& segment)
  : m_geometry(geometry)
  , m_segment(segment)
{
}

PathPoint::PathPoint(Segment& segment)
  : m_segment(segment)
{
}

std::set<PathPoint*> PathPoint::joined_points() const
{
  return path()->scene()->joined_points().get(this);
}

void PathPoint::join(std::set<PathPoint*> buddies)
{
  buddies.insert(this);
  path()->scene()->joined_points().insert(buddies);
}

void PathPoint::disjoin()
{
  path()->scene()->joined_points().remove({this});
}

Path* PathPoint::path() const
{
  return m_segment.path();
}

std::size_t PathPoint::index() const
{
  assert(path() != nullptr);
  std::size_t offset = 0;
  for (auto* segment : path()->segments()) {
    if (segment->contains(*this)) {
      return offset + segment->find(*this);
    } else {
      offset += segment->size();
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

PathPoint PathPoint::copy(Segment& segment) const
{
  return PathPoint(m_geometry, segment);
}

Segment& PathPoint::segment() const
{
  return m_segment;
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
