#include "path/facedetector.h"
#include "common.h"
#include "geometry/polarcoordinates.h"
#include "logging.h"
#include "path/face.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"
#include "path/pathvectorview.h"
#include "removeif.h"
#include "transform.h"
#include <set>
#include <vector>

namespace
{

omm::Direction get_direction(const omm::Edge& edge, const omm::PathPoint& start)
{
  if (edge.a().get() == &start) {
    return omm::Direction::Forward;
  } else if (edge.b().get() == &start) {
    return omm::Direction::Backward;
  } else {
    throw std::runtime_error("Unexpected condition.");
  }
}

class CCWComparator
{
public:
  explicit CCWComparator(const omm::DEdge& base) : m_base(base), m_base_arg(base.end_angle())
  {
  }

  [[nodiscard]] bool operator()(const omm::DEdge& a, const omm::DEdge& b) const noexcept
  {
    auto a_arg = omm::python_like_mod(a.start_angle() - m_base_arg, 2 * M_PI);
    auto b_arg = omm::python_like_mod(b.start_angle() - m_base_arg, 2 * M_PI);
    return a_arg < b_arg;
  }

private:
  omm::DEdge m_base;
  double m_base_arg;
};

}  // namespace

namespace omm
{

FaceDetector::FaceDetector(Graph graph) : m_graph(std::move(graph))
{
  for (auto* e : m_graph.edges()) {
    m_edges.emplace(e, Direction::Backward);
    m_edges.emplace(e, Direction::Forward);
  }

  while (!m_edges.empty()) {
    auto current = m_edges.extract(m_edges.begin()).value();
    std::deque<DEdge> sequence{current};

    const PathPoint* const start_point = &current.start_point();
    while (true) {
      auto const next = find_next_edge(current);
      // TODO what about single-edge loops?
      if (const auto v = m_edges.extract(next); v.empty()) {
        break;
      } else {
        sequence.emplace_back(next);
      }
      if (&next.end_point() == start_point) {
        m_faces.emplace(PathVectorView(sequence));
        break;
      }
      current = next;
    }
  }
}

const std::set<Face>& FaceDetector::faces() const
{
  return m_faces;
}

DEdge FaceDetector::find_next_edge(const DEdge& current) const
{
  const auto& hinge = current.end_point();
  const auto edges = m_graph.adjacent_edges(hinge);
  std::set<DEdge> candidates;
  for (Edge* e : edges) {
    if (e == current.edge) {
      continue;  // the next edge cannot be the current edge.
    }
    const auto direction = get_direction(*e, hinge);
    if (const DEdge dedge{e, direction}; m_edges.contains(dedge)) {
      candidates.emplace(dedge);
    }
  }

  const CCWComparator compare(current);
  const auto min_it = std::min_element(candidates.begin(), candidates.end(), compare);
  if (min_it == candidates.end()) {
    return {};
  } else {
    return *min_it;
  }
}

Graph::Graph(const PathVector& path_vector) : m_edges(util::transform<std::set>(path_vector.edges()))
{
  for (auto* edge : m_edges) {
    m_adjacent_edges[edge->a().get()].insert(edge);
    m_adjacent_edges[edge->b().get()].insert(edge);
  }
}

void Graph::remove_edge(Edge* edge)
{
  m_edges.erase(edge);

  for (auto* const p : edge->points()) {
    const auto it = m_adjacent_edges.find(p);
    it->second.erase(edge);
    if (it->second.empty()) {
      m_adjacent_edges.erase(it);
    }
  }
}

const std::set<Edge*>& Graph::edges() const
{
  return m_edges;
}

const std::set<Edge*>& Graph::adjacent_edges(const PathPoint& p) const
{
  return m_adjacent_edges.at(&p);
}

void Graph::remove_dead_ends()
{
  const auto is_not_on_fringe = [this](const auto& edge) { return degree(*edge->a()) != 1 && degree(*edge->b()) != 1; };
  std::set<Edge*> fringe = util::remove_if(m_edges, is_not_on_fringe);

  const auto add_to_fringe = [this, &fringe, &is_not_on_fringe](const PathPoint& p) {
    const auto edges = util::remove_if(m_adjacent_edges.at(&p), is_not_on_fringe);
    fringe.insert(edges.begin(), edges.end());
  };

  while (!fringe.empty()) {
    auto* const current = fringe.extract(fringe.begin()).value();
    remove_edge(*current);
    const auto i1 = m_adjacent_edges.find(current->a().get());
    const auto i2 = m_adjacent_edges.find(current->b().get());
    if (i1 != m_adjacent_edges.end() && i2 != m_adjacent_edges.end()) {
      // current was not on fringe, impossible.
    } else if (i1 != m_adjacent_edges.end()) {
      add_to_fringe(*current->a());
    } else if (i2 != m_adjacent_edges.end()) {
      add_to_fringe(*current->b());
    } else {
      // may happen if current edge is lonely
    }
  }
}

std::list<Graph> Graph::connected_components() const
{
  return {*this};  // TODO
}

std::size_t Graph::degree(const PathPoint& p) const
{
  const auto it = m_adjacent_edges.find(&p);
  if (it == m_adjacent_edges.end()) {
    return 0;
  } else {
    return it->second.size();
  }
}

void Graph::remove_edge(Edge& edge)
{
  m_edges.erase(&edge);
  for (const auto& p : {edge.a(), edge.b()}) {
    const auto it = m_adjacent_edges.find(p.get());
    it->second.erase(&edge);
    if (it->second.empty()) {
      m_adjacent_edges.erase(it);
    }
  }
}

}  // namespace omm
