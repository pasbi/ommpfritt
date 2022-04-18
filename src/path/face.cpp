#include "path/face.h"
#include "common.h"
#include "geometry/point.h"
#include "path/edge.h"
#include "path/path.h"
#include "path/pathgeometry.h"
#include "path/pathpoint.h"
#include <QPainterPath>
#include <QStringList>

namespace omm
{

std::vector<Point> Face::points() const
{
  assert(is_valid());
  if (empty()) {
    return {};
  }

  std::vector<Point> points;
  points.reserve(m_edges.size());
  for (const auto& edge : edges()) {
    points.emplace_back(edge->a()->geometry());
  }
  return points;
}

std::vector<PathPoint*> Face::path_points() const
{
  assert(is_valid());
  if (empty()) {
    return {};
  }

  std::vector<PathPoint*> points;
  points.reserve(m_edges.size() + 1);
  points.emplace_back(m_edges.front()->a().get());
  for (const auto& edge : m_edges) {
    points.emplace_back(edge->b().get());
  }
  return points;
}

QPainterPath Face::to_painter_path() const
{
  return PathGeometry{points()}.to_painter_path();
}

const std::deque<Edge*>& Face::edges() const
{
  return m_edges;
}

double Face::compute_aabb_area() const
{
  if (empty()) {
    return 0.0;
  }

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

  return (right - left) * (top - bottom);
}

QString Face::to_string() const
{
  const auto edges = util::transform<QList>(m_edges, std::mem_fn(&Edge::label));
  return static_cast<QStringList>(edges).join(", ");
}

bool Face::is_valid() const noexcept
{
  if (empty()) {
    return true;
  }
  if (!Path::is_valid(m_edges)) {
    return false;
  }
  return m_edges.front()->a() == m_edges.back()->b();
}

bool Face::empty() const noexcept
{
  return m_edges.empty();
}

std::size_t Face::size() const noexcept
{
  return m_edges.size();
}

bool Face::contains(const Face& other) const
{
  const auto ps_other = other.path_points();
  const auto ps_this = path_points();
  const auto pp = to_painter_path();

  std::set<const PathPoint*> distinct_points;
  const auto other_point_not_outside = [&pp, &ps_this](const auto* p_other) {
    const auto is_same = [p_other](const auto* p_this) { return p_other == p_this; };
    return std::any_of(ps_this.begin(), ps_this.end(), is_same) || pp.contains(p_other->geometry().position().to_pointf());
  };

  return std::all_of(ps_other.begin(), ps_other.end(), other_point_not_outside);
}

bool Face::contains(const Vec2f& pos) const
{
  return to_painter_path().contains(pos.to_pointf());
}

bool Face::operator==(const Face& other) const
{
  const auto& os = other.edges();
  const auto& ts = this->edges();
  if (os.size() != ts.size()) {
    return false;
  }
  if (os.size() == 0) {
    return true;
  }

  const auto eq = [&os, &ts](const auto& f) {
    for (std::size_t i = 0; i < os.size(); ++i) {
      if (os[i] != ts[f(i)]) {
        return false;
      }
    }
    return true;
  };

  for (std::size_t offset = 0; offset < os.size(); ++offset) {
    const auto f_offset_fwd = [offset, n = os.size()](const std::size_t i) { return (i + offset) % n; };
    const auto f_offset_bwd = [offset, n = os.size()](const std::size_t i) { return n - ((i + offset) % n); };
    if (eq(f_offset_fwd) || eq(f_offset_bwd)) {
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
  return m_edges < other.m_edges;
}

void Face::normalize()
{
  if (empty()) {
    const auto min_element = std::min_element(m_edges.begin(), m_edges.end());
    std::rotate(m_edges.begin(), min_element, m_edges.end());
  }
}

}  // namespace omm
