#include "path/facedetector.h"
#include "common.h"
#include "path/face.h"
#include "path/pathpoint.h"
#include "path/pathvectorview.h"
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

  [[nodiscard]] double angle_to(const omm::DEdge& edge) const noexcept
  {
    return omm::python_like_mod(edge.start_angle() - m_base_arg, 2 * M_PI);
  }

  [[nodiscard]] bool operator()(const omm::DEdge& a, const omm::DEdge& b) const noexcept
  {
    return angle_to(a) < angle_to(b);
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

    const auto is_face_done = [&sequence, this,
                               start_point = &current.start_point()](const PathPoint& current_end_point) {
      if (&current_end_point != start_point) {
        return false;
      }
      m_faces.emplace(PathVectorView(sequence));
      return true;
    };

    while (!is_face_done(current.end_point())) {
      auto const next = find_next_edge(current);
      [[maybe_unused]] const auto v = m_edges.extract(next);
      assert(!v.empty());  // The graph must not have dead ends! Every edge must be in exactly two faces.
      current = sequence.emplace_back(next);
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

  const CCWComparator compare_with_current(current);

  const auto min_it = std::min_element(candidates.begin(), candidates.end(), compare_with_current);
  if (min_it == candidates.end()) {
    return {};
  } else {
    return *min_it;
  }
}

}  // namespace omm
