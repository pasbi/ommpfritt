#pragma once

#include "path/dedge.h"
#include "path/edge.h"
#include <list>
#include <map>
#include <set>

namespace omm
{

class Face;
class PathPoint;
class PathVector;

class Graph
{
public:
  explicit Graph(const PathVector& path_vector);
  void remove_edge(Edge* edge);
  [[nodiscard]] const std::set<Edge*>& edges() const;
  [[nodiscard]] const std::set<Edge*>& adjacent_edges(const PathPoint& p) const;

  void remove_dead_ends();
  std::list<Graph> connected_components() const;
  std::size_t degree(const PathPoint& p) const;
  void remove_edge(Edge& edge);

private:
  std::set<Edge*> m_edges;
  std::map<const PathPoint*, std::set<Edge*>> m_adjacent_edges;
};

class FaceDetector
{
public:
  explicit FaceDetector(Graph graph);
  const std::set<Face>& faces() const;
  DEdge find_next_edge(const DEdge& dedge) const;

private:
  Graph m_graph;
  std::set<DEdge> m_edges;
  std::set<Face> m_faces;
};

}  // namespace omm
