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
  return p1 == p2 || (p1 != nullptr && p1->joined_points().contains(p2));
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

template<typename Ts, typename Rs>
bool equal_at_offset(const Ts& ts, const Rs& rs, const std::size_t offset)
{
  if (ts.size() != rs.size()) {
    return false;
  }

  for (std::size_t i = 0; i < ts.size(); ++i) {
    const auto j = (i + offset) % ts.size();
    if (!same_point(ts.at(i), rs.at(j))) {
      return false;
    }
  }
  return true;
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

std::deque<PathPoint*> Face::path_points() const
{
  std::deque<PathPoint*> points;
  for (const auto& edge : edges()) {
    points.emplace_back(edge.start_point());
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

bool operator==(const Face& a, const Face& b)
{
  const auto points_a = a.path_points();
  const auto points_b = b.path_points();
  if (points_a.size() != points_b.size()) {
    return false;
  }
  const auto points_b_reversed = std::deque(points_b.rbegin(), points_b.rend());
  QStringList pa;
  QStringList pb;
  for (std::size_t i = 0; i < points_a.size(); ++i) {
    pa.append(QString{"%1"}.arg(points_a.at(i)->index()));
    pb.append(QString{"%1"}.arg(points_b.at(i)->index()));
  }

  for (std::size_t offset = 0; offset < points_a.size(); ++offset) {
    if (equal_at_offset(points_a, points_b, offset)) {
      return true;
    }
    if (equal_at_offset(points_a, points_b_reversed, offset)) {
      return true;
    }
  }
  return false;
}

}  // namespace omm
