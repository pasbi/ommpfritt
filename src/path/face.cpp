#include "path/face.h"
#include "common.h"
#include "geometry/point.h"
#include "path/pathpoint.h"
#include "path/edge.h"
#include <QStringList>

namespace
{

using namespace omm;

bool same_point(const PathPoint* p1, const PathPoint* p2)
{
  return p1 == p2 || p1->joined_points().contains(p2);
}

bool align_last_edge(const Edge& second_last, Edge& last)
{
  assert(!last.flipped);
  if (same_point(second_last.end_point(), last.b)) {
    last.flipped = true;
    return true;
  } else {
    return same_point(second_last.end_point(), last.a);
  }
}

bool align_two_edges(Edge& second_last, Edge& last)
{
  assert(!last.flipped);
  assert(!second_last.flipped);
  if (same_point(second_last.b, last.b)) {
    last.flipped = true;
    return true;
  } else if (same_point(second_last.a, last.a)) {
    second_last.flipped = true;
    return true;
  } else if (same_point(second_last.a, last.b)) {
    second_last.flipped = true;
    last.flipped = true;
    return true;
  } else {
    return same_point(second_last.b, last.a);
  }
}

}  // namespace

namespace omm
{

std::list<Point> Face::points() const
{
  std::list<Point> points;
  for (const auto& edge : edges()) {
    if (points.empty()) {
      points.emplace_back(edge.start_geometry());
    } else {
      points.back().set_right_position(edge.start_geometry().right_position());
    }
    points.emplace_back(edge.end_geometry());
  }
  return points;
}

Face::~Face() = default;

bool Face::add_edge(const Edge& edge)
{
  assert(!edge.flipped);
  m_edges.emplace_back(edge);
  if (m_edges.size() == 2) {
    return align_two_edges(m_edges[0], m_edges[1]);
  } else if (m_edges.size() > 2) {
    return align_last_edge(m_edges[m_edges.size() - 2], m_edges.back());
  }
  return true;
}

const std::deque<Edge>& Face::edges() const
{
  return m_edges;
}

double Face::compute_aabb_area() const
{
  double left = std::numeric_limits<double>::infinity();
  double right = -std::numeric_limits<double>::infinity();
  double top = -std::numeric_limits<double>::infinity();
  double bottom = std::numeric_limits<double>::infinity();

  const auto points = this->points();
  for (const auto& p : points) {
    left = std::min(left, p.position().x);
    right = std::max(right, p.position().x);
    top = std::max(top, p.position().y);
    bottom = std::min(bottom, p.position().y);
  }

  if (points.empty()) {
    return 0.0;
  } else {
    return (right - left) * (top - bottom);
  }
}

QString Face::to_string() const
{
  const auto edges = util::transform<QList>(m_edges, std::mem_fn(&Edge::label));
  return static_cast<QStringList>(edges).join(", ");
}

}  // namespace omm
