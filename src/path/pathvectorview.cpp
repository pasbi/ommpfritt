#include "path/pathvectorview.h"
#include "path/dedge.h"
#include "path/edge.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include <QPainterPath>


namespace
{

std::size_t count_distinct_points(const omm::Edge& first, const omm::Edge& second)
{
  const auto* const a1 = first.a().get();
  const auto* const b1 = first.b().get();
  const auto* const a2 = second.a().get();
  const auto* const b2 = second.b().get();

  const auto n = std::set{a1, b1, a2, b2}.size();
  return n;
}

}  // namespace

namespace omm
{

PathVectorView::PathVectorView(std::deque<DEdge> edges) : m_edges(std::move(edges))
{
  assert(is_valid());
}

bool PathVectorView::is_valid() const
{
  static constexpr auto is_valid = [](const DEdge& de) { return de.edge == nullptr || !de.edge->is_valid(); };
  if (std::any_of(m_edges.begin(), m_edges.end(), is_valid)) {
    return false;
  }

  switch (m_edges.size()) {
  case 0:
    [[fallthrough]];
  case 1:
    return true;
  case 2:
    return count_distinct_points(*m_edges.front().edge, *m_edges.back().edge) <= 3;
  default:
    for (std::size_t i = 1; i < m_edges.size(); ++i) {
      const auto& current_edge = *m_edges.at(i).edge;
      const auto& previous_edge = *m_edges.at(i - 1).edge;
      const auto loop_count = static_cast<std::size_t>((current_edge.is_loop() ? 1 : 0)
                                                       + (previous_edge.is_loop() ? 1 : 0));
      if (count_distinct_points(current_edge, previous_edge) != 3 - loop_count) {
        return false;
      }
    }
    return true;
  }
}

bool PathVectorView::is_simply_closed() const
{
  switch (m_edges.size()) {
  case 0:
    return false;
  case 1:
    return m_edges.front().edge->a() == m_edges.front().edge->b();  // edge loops from point to itself
  case 2:
    // Both edges have the same points.
    // They can be part of different paths, hence any direction is possible.
    return count_distinct_points(*m_edges.front().edge, *m_edges.back().edge) == 2;
  default:
    // Assuming there are no intersections,
    // there must be only one common point between first and last edge to be closed.
    return count_distinct_points(*m_edges.front().edge, *m_edges.back().edge) == 3;
  }

  return !m_edges.empty() && m_edges.front().edge->a().get() == m_edges.back().edge->b().get();
}

const std::deque<DEdge>& PathVectorView::edges() const
{
  return m_edges;
}

QPainterPath PathVectorView::to_painter_path() const
{
  assert(is_valid());
  if (m_edges.empty()) {
    return {};
  }
  QPainterPath p;
  p.moveTo(m_edges.front().start_point().geometry().position().to_pointf());
  for (const auto& edge : m_edges) {
    auto* const path = edge.edge->path();
    const auto g1 = edge.start_point().geometry();
    const auto g2 = edge.end_point().geometry();
    auto bwd = Direction::Backward;
    auto fwd = Direction::Forward;
    if (edge.direction == Direction::Forward) {
      std::swap(bwd, fwd);
    }
    p.cubicTo(g1.tangent_position({path, bwd}).to_pointf(), g2.tangent_position({path, fwd}).to_pointf(),
              g2.position().to_pointf());
  }

  return p;
}

Geom::Path PathVectorView::to_geom() const
{
  Geom::Path path;
  for (const auto& dedge : m_edges) {
    path.append(dedge.to_geom_curve().release());
  }
  return path;
}

bool PathVectorView::contains(const Vec2f& pos) const
{
  return to_painter_path().contains(pos.to_pointf());
}

QString PathVectorView::to_string() const
{
  static constexpr auto label = [](const auto& dedge) { return dedge.edge->label(); };
  const auto es = static_cast<QStringList>(util::transform<QList>(edges(), label)).join(", ");
  return QString("[%1] %2").arg(edges().size()).arg(es);
}

std::vector<PathPoint*> PathVectorView::path_points() const
{
  if (m_edges.empty()) {
    return {};
  }

  std::vector<PathPoint*> ps;
  ps.reserve(m_edges.size() + 1);
  ps.emplace_back(&m_edges.front().start_point());
  for (std::size_t i = 0; i < m_edges.size(); ++i) {
    ps.emplace_back(&m_edges.at(i).end_point());
  }
  return ps;
}

QRectF PathVectorView::bounding_box() const
{
  static constexpr auto get_geometry = [](const auto* const pp) { return pp->geometry(); };
  return Point::bounding_box(util::transform<std::list>(path_points(), get_geometry));
}

std::vector<Vec2f> PathVectorView::bounding_polygon() const
{
  std::vector<Vec2f> poly;

  // each edge can add at most three points since the end point of one edge is
  // the start point of the next one and the sequence of edges is closed.
  poly.reserve(m_edges.size() * 3);

  for (const auto& edge : m_edges) {
    poly.emplace_back(edge.start_point().geometry().position());
    const auto add_tangent_maybe = [&poly, path = edge.edge->path()](const Point& p, const auto& direction) {
      static constexpr auto eps = 0.001;
      const Point::TangentKey key{path, direction};
      if (p.tangent(key).magnitude > eps) {
        poly.emplace_back(p.tangent_position(key));
      }
    };
    add_tangent_maybe(edge.start_point().geometry(), Direction::Forward);
    add_tangent_maybe(edge.end_point().geometry(), Direction::Backward);
  }
  poly.shrink_to_fit();
  return poly;
}

std::vector<Edge*> PathVectorView::normalized() const
{
  auto edges = util::transform<std::vector>(m_edges, [](const auto& dedge) { return dedge.edge; });
  if (is_simply_closed()) {
    const auto min_it = std::min_element(edges.begin(), edges.end());
    std::rotate(edges.begin(), min_it, edges.end());
    if (edges.size() >= 3 && edges.at(1) > edges.back()) {
      std::reverse(edges.begin(), edges.end());
      // Reversing has moved the smallest edge to the end, but it must be at front after
      // normalization .
      std::rotate(edges.begin(), std::prev(edges.end()), edges.end());
    }
  } else if (edges.size() >= 2 && edges.at(0) > edges.at(1)) {
    std::reverse(edges.begin(), edges.end());
  }
  return edges;
}

bool operator==(const PathVectorView& a, const PathVectorView& b)
{
  return a.normalized() == b.normalized();
}

bool operator<(const PathVectorView& a, const PathVectorView& b)
{
  return a.normalized() < b.normalized();
}

}  // namespace omm
