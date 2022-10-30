#pragma once

#include "graph.h"
#include <list>
#include <set>

namespace omm
{

class Face;

namespace face_detector
{

[[nodiscard]] std::set<Face> compute_faces_without_outer(Graph graph);
[[nodiscard]] std::set<Face> compute_faces_on_connected_graph_without_dead_ends(const Graph& graph);
[[nodiscard]] Face find_outer_face(const std::set<Face>& faces);

}  // namespace face_detector

}  // namespace omm
