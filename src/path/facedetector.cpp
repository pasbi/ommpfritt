#include <vector>
#include <set>
#include "geometry/polarcoordinates.h"
#include "path/edge.h"
#include "path/pathpoint.h"
#include "path/face.h"
#include "common.h"
#include "transform.h"
#include "path/pathvector.h"
#include "path/pathvectorview.h"
#include "logging.h"


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

omm::Edge* find_next_edge(const omm::PathPoint& hinge, const omm::Edge& arm)
{
  auto edges = hinge.edges();
  assert(!edges.empty());
  std::vector<omm::Edge*> edges_v;
  edges_v.reserve(edges.size() - 1);
  const auto is_not_arm = [&arm](const auto* const c) { return c != &arm; };
  std::copy_if(edges.begin(), edges.end(), std::back_inserter(edges_v), is_not_arm);
  assert(edges.size() == edges_v.size() + 1);
  if (edges_v.empty()) {
    return nullptr;
  }

  static constexpr auto pi2 = 2.0 * M_PI;
  const auto ref_arg = omm::python_like_mod(get_direction_at(arm, hinge).argument, pi2);
  const auto ccw_angle_to_arm = util::transform(edges_v, [&hinge, ref_arg](const auto* const edge) {
    const auto target_arg = omm::python_like_mod(get_direction_at(*edge, hinge).argument, pi2);
    return omm::python_like_mod(target_arg - ref_arg, pi2);
  });
  const auto min_it = std::min_element(ccw_angle_to_arm.begin(), ccw_angle_to_arm.end());
  const auto i = std::distance(ccw_angle_to_arm.begin(), min_it);
  return edges_v.at(i);
}

}  // namespace

namespace omm
{

std::set<Face> detect_faces(const PathVector& path_vector)
{
  // implements this suggestion: https://mathoverflow.net/a/23958
  std::map<bool, std::set<Edge*>> todo_edges{
    // The graph is considered undirected, so each edge is both flipped and unflipped.
      {false, util::transform<std::set>(path_vector.edges())},  // unflipped edges
      {true, util::transform<std::set>(path_vector.edges())},   // flipped edges
  };

  const auto follow = [&todo_edges](Edge* const edge, bool flipped) -> PathVectorView {
    std::deque<Edge*> sequence{edge};
    while (true) {
      [[maybe_unused]] const auto deleted_edges = todo_edges[flipped].erase(sequence.back());
      if (deleted_edges == 0) {
        // we've reached an already visited edge.
        // This can only happen in parts of the graph that don't form a face.
        return PathVectorView();  // those edges don't form a face.
      }
      assert(deleted_edges == 1);

      const auto& hinge = *(flipped ? sequence.back()->a() : sequence.back()->b());
      auto* const current_edge = find_next_edge(hinge, *sequence.back());
      if (current_edge == nullptr) {
        return PathVectorView();  // those edges don't form a face.
      }

      flipped = current_edge->b().get() == &hinge;
      if (current_edge == edge) {
        // The current face is complete.
        return PathVectorView(sequence);
      }
      sequence.emplace_back(current_edge);
    }
  };

  std::set<Face> faces;
  for (auto& [direction, edges] : todo_edges) {
    while (!edges.empty()) {
      Edge* const current_edge = *edges.begin();
      auto face = follow(current_edge, direction);
      if (!face.edges().empty()) {
        faces.emplace(std::move(face));
      }
    }
  }
  return faces;
}

}  // namespace omm
