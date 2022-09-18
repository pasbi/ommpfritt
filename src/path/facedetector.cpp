#include "path/facedetector.h"
#include "geometry/polarcoordinates.h"
#include "path/pathpoint.h"
#include "path/face.h"
#include "common.h"
#include "transform.h"
#include "path/pathvector.h"
#include "path/pathvectorview.h"
#include "logging.h"
#include <vector>
#include <set>


namespace
{

omm::PolarCoordinates get_direction_at(const omm::Edge& edge, const omm::PathPoint& vertex)
{
  const auto d = &vertex == edge.a().get() ? omm::Point::Direction::Forward : omm::Point::Direction::Backward;
  const auto& tangent = vertex.geometry().tangent({edge.path(), d});
  static constexpr auto eps = 1e-2;
  if (tangent.magnitude < eps) {
    return omm::PolarCoordinates(edge.b()->geometry().position() - edge.a()->geometry().position());
  } else {
    return tangent;
  }
}

}  // namespace

namespace omm
{

FaceDetector::FaceDetector(const PathVector& path_vector)
    : m_graph(path_vector)
{
  m_unvisited_edges = {
    {Edge::Direction::Forward, m_graph.edges()},
    {Edge::Direction::Backward, m_graph.edges()},
  };
  LINFO << "Face detector";
  for (auto& [direction, edges] : m_unvisited_edges) {
    LINFO << "  direction " << (direction == Edge::Direction::Forward ? "fwd" : "bwd");
    while (!edges.empty()) {
      LINFO << "    new face";
      follow(*edges.begin(), direction);
    }
  }
}

const std::set<Face>& FaceDetector::faces() const
{
  return m_faces;
}

Edge* FaceDetector::find_next_edge(const omm::PathPoint& hinge, omm::Edge* arm) const
{
  auto edges = m_graph.adjacent_edges(hinge);
  assert(!edges.empty());
  [[maybe_unused]] const auto deleted_edge_count = edges.erase(arm);
  assert(deleted_edge_count == 1);
  auto edges_v = std::vector(edges.begin(), edges.end());
  if (edges_v.empty()) {
    return nullptr;
  }

  static constexpr auto pi2 = 2.0 * M_PI;
  const auto ref_arg = omm::python_like_mod(get_direction_at(*arm, hinge).argument, pi2);
  const auto ccw_angle_to_arm = util::transform(edges_v, [&hinge, ref_arg](const auto* const edge) {
    const auto target_arg = omm::python_like_mod(get_direction_at(*edge, hinge).argument, pi2);
    return omm::python_like_mod(target_arg - ref_arg, pi2);
  });
  const auto min_it = std::min_element(ccw_angle_to_arm.begin(), ccw_angle_to_arm.end());
  const auto i = std::distance(ccw_angle_to_arm.begin(), min_it);
  return edges_v.at(i);
}

bool FaceDetector::follow(Edge* edge, Edge::Direction direction)
{
  LINFO << "      add " << edge->label();
  std::deque<Edge*> sequence{edge};
  while (true) {
    auto& current_edge = *sequence.back();
    const auto deleted_edges = m_unvisited_edges.at(direction).erase(&current_edge);
    if (deleted_edges == 0) {
      // we've reached an already visited edge.
      // This can only happen in parts of the graph that don't form a face.
      LINFO << "      visit edge " << sequence.back()->label() << " again. Hence no face.";
      return false;
    }

    const auto& hinge = *current_edge.end_point(direction);
    auto* const next_edge = find_next_edge(hinge, sequence.back());
    if (next_edge == nullptr) {
      LINFO << "      edge " << sequence.back()->label() << " doesn't have a follower. Hence no face.";
      return false;
    }

    if (next_edge == edge) {
      // The current face is complete.
      LINFO << "      complete.";
      m_faces.emplace(PathVectorView(sequence));
      return true;
    }
    LINFO << "      add " << next_edge->label();
    sequence.emplace_back(next_edge);
    direction = next_edge->a().get() == &hinge ? Edge::Direction::Forward : Edge::Direction::Backward;
  }
}

Graph::Graph(const PathVector& path_vector)
    : m_edges(util::transform<std::set>(path_vector.edges()))
{
  for (auto* edge : m_edges) {
    m_adjacent_edges[edge->a().get()].insert(edge);
    m_adjacent_edges[edge->b().get()].insert(edge);
  }
}

void Graph::remove_edge(Edge* edge)
{
  m_edges.erase(edge);

  for (auto* const p : {edge->a().get(), edge->b().get()}) {
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

}  // namespace omm
