#pragma once

#include "path/dedge.h"
#include <list>
#include <map>
#include <set>

namespace omm
{

class PathPoint;
class PathVector;

class Graph
{
public:
  explicit Graph(const PathVector& path_vector);
  explicit Graph() = default;
  void remove_edge(Edge* edge);
  [[nodiscard]] const std::set<Edge*>& edges() const;
  [[nodiscard]] const std::set<DEdge>& out_edges(const PathPoint& from) const;

  void remove_dead_ends();
  [[nodiscard]] std::list<Graph> connected_components() const;
  void remove_edge(Edge& edge);
  [[nodiscard]] std::set<PathPoint*> vertices() const;
  [[nodiscard]] std::set<PathPoint*> connected_component(PathPoint& seed) const;
  [[nodiscard]] Graph subgraph(const std::set<PathPoint*>& vertices) const;
  [[nodiscard]] std::size_t degree(const PathPoint& p) const;

private:
  std::set<Edge*> m_edges;
  std::map<const PathPoint*, std::set<DEdge>> m_out_edges;
  std::map<const PathPoint*, std::set<Edge*>> m_adjacent_edges;
};

}  // namespace omm
