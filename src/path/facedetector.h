#pragma once

#include "path/edge.h"
#include <set>
#include <map>


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
//  void remove_bridges();
  [[nodiscard]] const std::set<Edge*>& edges() const;
  [[nodiscard]] const std::set<Edge*>& adjacent_edges(const PathPoint& p) const;

private:
  std::set<Edge*> m_edges;
  std::map<const PathPoint*, std::set<Edge*>> m_adjacent_edges;
};

class FaceDetector
{
public:
  explicit FaceDetector(const PathVector& path_vector);
  const std::set<Face>& faces() const;
  Edge* find_next_edge(const PathPoint& hinge, Edge* arm) const;
  bool follow(Edge* edge, Edge::Direction direction);

private:
  Graph m_graph;
  std::map<Edge::Direction, std::set<Edge*>> m_unvisited_edges;
  std::set<Face> m_faces;
};

}  // namespace omm
