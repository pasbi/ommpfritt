#include "commands/ownedlocatedpath.h"

#include "path/edge.h"
#include "path/path.h"

#include <algorithm>


namespace omm
{

OwnedLocatedPath::OwnedLocatedPath(Path* const path,
                                   const std::size_t point_offset,
                                   std::deque<std::shared_ptr<PathPoint>> points)
    : m_path(path)
    , m_point_offset(point_offset)
    , m_points(std::move(points))
{
  assert(std::none_of(m_points.begin(), m_points.end(), [](const auto& p) { return p.get() == nullptr; }));
}

OwnedLocatedPath::~OwnedLocatedPath() = default;

std::deque<std::unique_ptr<Edge>> OwnedLocatedPath::create_edges() const
{
  std::deque<std::unique_ptr<Edge>> edges;
  for (std::size_t i = 1; i < m_points.size(); ++i) {
    edges.emplace_back(std::make_unique<Edge>(std::move(m_points[i - 1]), std::move(m_points[i]), m_path));
  }

  std::shared_ptr<PathPoint> front = edges.empty() ? m_points.front() : edges.front()->a();
  std::shared_ptr<PathPoint> back = edges.empty() ? m_points.back() : edges.back()->b();

  if (m_point_offset > 0) {
    // if there is something left of this, add the linking edge
    std::shared_ptr<PathPoint> right_fringe;
    if (m_path->edges().empty()) {
      right_fringe = m_path->last_point();
    } else if (m_point_offset > 1) {
      right_fringe = m_path->edges()[m_point_offset - 2]->b();
    } else {
      right_fringe = m_path->edges()[m_point_offset - 1]->a();
    }
    edges.emplace_front(std::make_unique<Edge>(right_fringe, front, m_path));
  }

  if (m_point_offset < m_path->points().size()) {
    // if there is something right of this, add the linking edge
    std::shared_ptr<PathPoint> left_fringe;
    if (m_path->edges().empty()) {
      left_fringe = m_path->first_point();
    } else if (m_point_offset > 0) {
      left_fringe = m_path->edges().at(m_point_offset - 1)->b();
    } else {
      left_fringe = m_path->edges().at(m_point_offset)->a();
    }
    edges.emplace_back(std::make_unique<Edge>(back, left_fringe, m_path));
  }

  assert(Path::is_valid(edges));
  return edges;
}

std::shared_ptr<PathPoint> OwnedLocatedPath::single_point() const
{
  if (m_points.size() == 1 && m_path->points().size() == 0) {
    return m_points.front();
  } else {
    return {};
  }
}

std::size_t OwnedLocatedPath::point_offset() const
{
  return m_point_offset;
}

Path* OwnedLocatedPath::path() const
{
  return m_path;
}

}  // namespace omm
