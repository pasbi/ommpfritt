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

template<typename Key, typename T> std::set<T> max_elements(const std::set<T>& vs, const Key& key)
{
  if (vs.empty()) {
    return {};
  }
  std::set<T> out{*vs.begin()};
  auto max_value = key(*vs.begin());
  for (auto it = std::next(vs.begin()); it != vs.end(); ++it) {
    const auto& v = *it;
    if (const auto value = key(v); value == max_value) {
      out.insert(v);
    } else if (value > max_value) {
      out = {v};
      max_value = value;
    }
  }
  return out;
}

}  // namespace

namespace omm::face_detector
{

std::set<Face> compute_faces_on_connected_graph_without_dead_ends(const Graph& graph)
{
  std::set<DEdge> edges;
  std::set<Face> faces;
  for (auto* e : graph.edges()) {
    edges.emplace(e, Direction::Backward);
    edges.emplace(e, Direction::Forward);
  }

  while (!edges.empty()) {
    auto current = edges.extract(edges.begin()).value();
    std::deque<DEdge> sequence{current};

    const auto is_face_done = [&sequence, &faces,
                               start_point = &current.start_point()](const PathPoint& current_end_point) {
      if (&current_end_point != start_point) {
        return false;
      }
      faces.emplace(PathVectorView(sequence));
      return true;
    };

    while (!is_face_done(current.end_point())) {
      auto const next = find_next_edge(current, graph, edges);
      [[maybe_unused]] const auto v = edges.extract(next);
      assert(!v.empty());  // The graph must not have dead ends! Every edge must be in exactly two faces.
      current = sequence.emplace_back(next);
    }
  }
  return faces;
}

DEdge find_next_edge(const DEdge& current, const Graph& graph, const std::set<DEdge>& white_list)
{
  const auto& hinge = current.end_point();
  const auto edges = graph.adjacent_edges(hinge);
  std::set<DEdge> candidates;
  for (Edge* e : edges) {
    if (e == current.edge) {
      continue;  // the next edge cannot be the current edge.
    }
    const auto direction = get_direction(*e, hinge);
    if (const DEdge dedge{e, direction}; white_list.contains(dedge)) {
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

Face find_outer_face(const std::set<Face>& faces)
{
  assert(!faces.empty());
  static constexpr auto bb_area = [](const auto& face) {
    const auto bb = face.path_vector_view().bounding_box();
    return bb.width() * bb.height();
  };

  // the outer face must have maximal bounding box area ...
  auto outers = max_elements(faces, bb_area);

  // ... however, multiple faces might have maximal bounding box area ...
  for (const auto& face : outers) {
    if (std::all_of(outers.begin(), outers.end(), [&face](const auto& f) { return face.contains(f); })) {
      return face;
    }
  }
  throw std::runtime_error("No face contains all other faces.");
}

std::set<Face> compute_faces_without_outer(Graph graph)
{
  graph.remove_dead_ends();

  std::set<Face> faces;
  for (const auto& connected_component : graph.connected_components()) {
    auto c_faces = compute_faces_on_connected_graph_without_dead_ends(connected_component);
    if (c_faces.size() > 1) {
      c_faces.erase(find_outer_face(c_faces));
    }
    faces.insert(c_faces.begin(), c_faces.end());
  }
  return faces;
}

}  // namespace omm::face_detector
