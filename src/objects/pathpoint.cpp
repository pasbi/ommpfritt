#include "objects/pathpoint.h"

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

void PathPoint::set_geometry(const Point& point)
{
  m_geometry = point;
}

const Point& PathPoint::geometry() const
{
  return m_geometry;
}

Point& PathPoint::geometry()
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

void PathPoint::set_selected(bool selected)
{
  m_is_selected = selected;
}

}  // namespace omm
