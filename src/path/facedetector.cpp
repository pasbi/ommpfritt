#include "path/facedetector.h"
#include "common.h"
#include "path/dedge.h"
#include "path/edge.h"
#include "path/face.h"
#include "path/pathpoint.h"
#include "path/pathvectorview.h"
#include <set>
#include <vector>

namespace
{

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

omm::DEdge find_next_edge(const omm::DEdge& current, const omm::Graph& graph, std::set<omm::DEdge>& allowed_edges)
{
  const auto& hinge = current.end_point();
  const auto edges = graph.out_edges(hinge);

  const CCWComparator compare_with_current(current);
  std::set<omm::DEdge> candidates;
  for (const auto& de : edges) {
    const auto is_current_reversed = current.edge == de.edge && current.direction != de.direction;
    if (!is_current_reversed && allowed_edges.contains(de)) {
      candidates.emplace(de);
    }
  }

  const auto min_it = std::min_element(candidates.begin(), candidates.end(), compare_with_current);
  if (min_it == candidates.end()) {
    return {};
  } else {
    auto next_edge = *min_it;
    allowed_edges.erase(next_edge);
    return next_edge;
  }
}

std::deque<omm::DEdge> follow_edge(const omm::DEdge& seed, const omm::Graph& graph, std::set<omm::DEdge>& allowed_edges)
{
  std::deque<omm::DEdge> sequence;
  auto next = seed;

  do {
    next = find_next_edge(next, graph, allowed_edges);
    if (next.edge == nullptr) {
      if (sequence.empty() && seed.edge->a() == seed.edge->b()) {
        allowed_edges.erase(seed);
        return {seed};
      }
      return {};
    }
    sequence.emplace_back(next);
  } while (next != seed);
  return sequence;
}

}  // namespace

namespace omm::face_detector
{

std::set<Face> compute_faces_on_connected_graph_without_dead_ends(const Graph& graph)
{
  std::set<DEdge> allowed_edges;
  std::set<Face> faces;
  for (auto* e : graph.edges()) {
    allowed_edges.emplace(e, Direction::Backward);
    allowed_edges.emplace(e, Direction::Forward);
  }

  while (!allowed_edges.empty()) {

    const auto current = *allowed_edges.begin();
    const auto sequence = follow_edge(current, graph, allowed_edges);

    if (!sequence.empty()) {
      faces.emplace(omm::PathVectorView(sequence));
    }
  }
  return faces;
}

auto argmax(const auto& values, const auto& key)
{
  using value_type = decltype(key(*values.begin()));
  auto max = -std::numeric_limits<value_type>::infinity();
  auto max_it = values.end();
  for (auto it = values.begin(); it != values.end(); ++it) {
    if (const auto v = key(*it); v >= max) {
      max_it = it;
      max = v;
    }
  }
  return max_it;
}

Face find_outer_face(const std::set<Face>& faces)
{
  assert(!faces.empty());
  const std::vector faces_v(faces.begin(), faces.end());
  return *argmax(faces, [](const auto& face) { return std::abs(face.area()); });
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
