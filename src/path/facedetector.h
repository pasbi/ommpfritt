#pragma once

#include "graph.h"
#include "path/dedge.h"
#include <list>
#include <map>
#include <optional>
#include <set>

namespace omm
{

class Face;

namespace face_detector
{

[[nodiscard]] std::set<Face> compute_faces_without_outer(Graph graph);
[[nodiscard]] std::set<Face> compute_faces_on_connected_graph_without_dead_ends(const Graph& graph);
[[nodiscard]] std::optional<Face> find_outer_face(const std::set<Face>& faces);
[[nodiscard]] DEdge find_next_edge(const DEdge& current, const Graph& graph, const std::set<DEdge>& white_list);

}  // namespace face_detector

}  // namespace omm
