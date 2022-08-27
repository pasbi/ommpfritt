#include "path/pathvectorview.h"
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

PathVectorView::PathVectorView(std::deque<Edge*> edges)
    : m_edges(std::move(edges))
{
  assert(is_valid());
}

bool PathVectorView::is_valid() const
{
  static constexpr auto is_valid = [](const Edge* const edge) { return edge == nullptr || !edge->is_valid(); };
  if (std::any_of(m_edges.begin(), m_edges.end(), is_valid)) {
    return false;
  }

  switch (m_edges.size()) {
  case 0:
    [[fallthrough]];
  case 1:
    return true;
  case 2:
    return count_distinct_points(*m_edges.front(), *m_edges.back()) <= 3;
  default:
    for (std::size_t i = 1; i < m_edges.size(); ++i) {
      if (count_distinct_points(*m_edges.at(i - 1), *m_edges.at(i)) != 3) {
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
    return m_edges.front()->a() == m_edges.front()->b();  // edge loops from point to itself
  case 2:
    // Both edges have the same points.
    // They can be part of different paths, hence any direction is possible.
    return count_distinct_points(*m_edges.front(), *m_edges.back()) == 2;
  default:
    // Assuming there are no intersections,
    // there must be only one common point between first and last edge to be closed.
    return count_distinct_points(*m_edges.front(), *m_edges.back()) == 3;
  }

  return !m_edges.empty() && m_edges.front()->a().get() == m_edges.back()->b().get();
}

const std::deque<Edge*>& PathVectorView::edges() const
{
  return m_edges;
}

QPainterPath PathVectorView::to_painter_path() const
{
  assert(is_valid());
  if (m_edges.empty()) {
    return {};
  }
  const auto ef = edge_flipped();
  QPainterPath p;
  p.moveTo([this, &ef]() {
    const auto& edge = *m_edges.front();
    const auto* const p = (ef.front() ? edge.b() : edge.a()).get();
    return p->geometry().position().to_pointf();
  }());
  for (std::size_t i = 0; i < m_edges.size(); ++i) {
    PathPoint* a = m_edges[i]->a().get();
    PathPoint* b = m_edges[i]->b().get();
    if (ef.at(i)) {
      std::swap(a, b);
    }
    Path::draw_segment(p, *a, *b, m_edges[i]->path());
  }

  return p;
}

std::vector<bool> PathVectorView::edge_flipped() const
{
  switch (m_edges.size()) {
  case 0:
    return {};
  case 1:
    return {false};
  default:
  {
    std::vector<bool> edge_flipped;
    edge_flipped.reserve(m_edges.size());
    edge_flipped.emplace_back(m_edges.at(1)->contains(m_edges.at(0)->b().get()));
    for (std::size_t i = 1; i < m_edges.size(); ++i) {
      edge_flipped.emplace_back(m_edges.at(i - 1)->contains(m_edges.at(i)->b().get()));
    }
    return edge_flipped;
  }
  }
}

bool PathVectorView::contains(const Vec2f& pos) const
{
  return to_painter_path().contains(pos.to_pointf());
}

std::vector<PathPoint*> PathVectorView::path_points() const
{
  if (m_edges.empty()) {
    return {};
  }

  std::vector<PathPoint*> ps;
  const auto flipped = edge_flipped();
  ps.push_back((flipped.front() ? m_edges.front()->b() : m_edges.front()->a()).get());
  for (std::size_t i = 0; i < m_edges.size(); ++i) {
    const auto& edge = *m_edges.at(i);
    ps.push_back((flipped.at(i) ? edge.a() : edge.b()).get());
  }
  return ps;
}

void PathVectorView::normalize()
{
  if (m_edges.empty()) {
    return;
  }

  if (is_simply_closed()) {
    const auto min_it = std::min_element(m_edges.begin(), m_edges.end());
    std::rotate(m_edges.begin(), min_it, m_edges.end());
  }

  if (m_edges.front() < m_edges.back()) {
    std::reverse(m_edges.begin(), m_edges.end());
  }
}

bool operator==(PathVectorView a, PathVectorView b)
{
  a.normalize();
  b.normalize();
  return a.edges() == b.edges();
}

bool operator<(PathVectorView a, PathVectorView b)
{
  a.normalize();
  b.normalize();
  return a.edges() < b.edges();
}

}  // namespace omm
