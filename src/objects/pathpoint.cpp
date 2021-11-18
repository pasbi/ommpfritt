#include "objects/pathpoint.h"
#include "objects/segment.h"
#include "objects/path.h"
#include "scene/scene.h"

namespace
{

class BlockJoinedPointsUpdateGuard
{
public:
  explicit BlockJoinedPointsUpdateGuard(bool& flag) noexcept : m_flag(flag) { m_flag = true; }
  BlockJoinedPointsUpdateGuard(const BlockJoinedPointsUpdateGuard&) = delete;
  BlockJoinedPointsUpdateGuard(BlockJoinedPointsUpdateGuard&&) = delete;
  BlockJoinedPointsUpdateGuard& operator=(const BlockJoinedPointsUpdateGuard&) = delete;
  BlockJoinedPointsUpdateGuard& operator=(BlockJoinedPointsUpdateGuard&&) = delete;
  ~BlockJoinedPointsUpdateGuard() { m_flag = false; }
private:
  bool& m_flag;
};

}  // namespace

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

void PathPoint::set_geometry(const Point& point)
{
  m_geometry = point;
  if (!m_block_joined_points_update) {
    for (PathPoint* q : path()->scene()->joined_points().get(this)) {
      auto geometry = q->geometry();
      geometry.set_position(this->geometry().position());
      BlockJoinedPointsUpdateGuard blocker{q->m_block_joined_points_update};
      q->set_geometry(geometry);
    }
  }
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

void PathPoint::set_selected(bool selected)
{
  m_is_selected = selected;
}

}  // namespace omm
