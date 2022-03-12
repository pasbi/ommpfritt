#include "path/face.h"
#include "common.h"
#include "geometry/point.h"
#include "path/edge.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include <QPainterPath>
#include <QStringList>

namespace
{

using namespace omm;

bool align_last_edge(const Edge& second_last, Edge& last)
{
  assert(!last.flipped);
  if (PathPoint::eq(second_last.end_point(), last.b)) {
    last.flipped = true;
    return true;
  } else {
    return PathPoint::eq(second_last.end_point(), last.a);
  }
}

bool align_two_edges(Edge& second_last, Edge& last)
{
  assert(!last.flipped);
  assert(!second_last.flipped);
  if (PathPoint::eq(second_last.b, last.b)) {
    last.flipped = true;
    return true;
  } else if (PathPoint::eq(second_last.a, last.a)) {
    second_last.flipped = true;
    return true;
  } else if (PathPoint::eq(second_last.a, last.b)) {
    second_last.flipped = true;
    last.flipped = true;
    return true;
  } else {
    return PathPoint::eq(second_last.b, last.a);
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
    if (!PathPoint::eq(ts.at(i), rs.at(j))) {
      return false;
    }
  }
  return true;
}

struct EdgePartition
{
  explicit EdgePartition(const std::deque<Edge>& as, const std::deque<Edge>& bs)
  {
    const auto set_a = std::set(as.begin(), as.end());
    const auto set_b = std::set(bs.begin(), bs.end());
    for (const auto& a : as) {
      if (set_b.contains(a)) {
        both.push_back(a);
      } else {
        only_a.push_back(a);
      }
    }
    for (const auto& b : bs) {
      if (!set_a.contains(b)) {
        only_b.push_back(b);
      }
    }
  }

  std::list<Edge> only_a;
  std::list<Edge> both;
  std::list<Edge> only_b;
};

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

QPainterPath Face::to_q_painter_path() const
{
  return Path::to_painter_path(points());
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

Face::Face(std::deque<Edge> edges)
    : m_edges(std::move(edges))
{
  assert(is_valid());
}

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

QPainterPath Face::to_painter_path() const
{
  return Path::to_painter_path(points());
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

bool Face::is_valid() const
{
  const auto n = m_edges.size();
  for (std::size_t i = 0; i < n; ++i) {
    if (!PathPoint::eq(m_edges[i].end_point(), m_edges[(i + 1) % n].start_point())) {
      return false;
    }
  }
  return true;
}

Face Face::operator^(const Face& other) const
{
  assert(contains(other));
  auto [only_a_edges, both_edges, only_b_edges] = EdgePartition{edges(), other.edges()};
  auto edges = std::deque(only_a_edges.begin(), only_a_edges.end());
  if (PathPoint::eq(only_a_edges.back().end_point(), only_b_edges.front().start_point())) {
    edges.insert(edges.end(), only_b_edges.begin(), only_b_edges.end());
  } else {
    for (auto& e : only_b_edges) {
      e.flipped = !e.flipped;
    }
    edges.insert(edges.end(), only_b_edges.rbegin(), only_b_edges.rend());
  }
  assert(PathPoint::eq(only_a_edges.back().end_point(), only_b_edges.front().start_point()));
  assert(PathPoint::eq(only_a_edges.front().start_point(), only_b_edges.back().end_point()));
  return Face{std::move(edges)};
}

Face& Face::operator^=(const Face& other)
{
  *this = *this ^ other;
  return *this;
}

bool Face::contains(const Face& other) const
{
  const QPainterPath p_this = Path::to_painter_path(points());
  const QPainterPath p_other = Path::to_painter_path(other.points());
  return p_this.contains(p_other);
}

bool Face::operator==(const Face& other) const
{
  const auto points = path_points();
  const auto other_points = other.path_points();
  if (points.size() != other_points.size()) {
    return false;
  }
  const auto other_points_reversed = std::deque(other_points.rbegin(), other_points.rend());

  for (std::size_t offset = 0; offset < points.size(); ++offset) {
    if (equal_at_offset(points, other_points, offset)) {
      return true;
    }
    if (equal_at_offset(points, other_points_reversed, offset)) {
      return true;
    }
  }
  return false;
}

bool Face::operator!=(const Face& other) const
{
  return !(*this == other);
}

bool Face::operator<(const Face& other) const
{
  const auto points = path_points();
  const auto other_points = other.path_points();
  if (points.size() != other_points.size()) {
    return points.size() < other_points.size();
  }

  for (std::size_t i = 0; i < points.size(); ++i) {
    const auto pindex = points.at(i)->index();
    const auto other_pindex = other_points.at(i)->index();
    if (pindex < other_pindex) {
      return pindex < other_pindex;
    }
  }

  return false;  // faces are equal
}

}  // namespace omm
