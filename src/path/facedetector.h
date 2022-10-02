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

  struct DEdge {
    explicit DEdge(Edge* edge, Direction direction);
    DEdge() = default;
    Edge* edge = nullptr;
    Direction direction = Direction::Forward;
    [[nodiscard]] bool operator<(const DEdge& other) const;
    [[nodiscard]] bool operator==(const DEdge& other) const;
    [[nodiscard]] const PathPoint& end_point() const;
    [[nodiscard]] const PathPoint& start_point() const;
    [[nodiscard]] double start_angle() const;
    [[nodiscard]] double end_angle() const;
    [[nodiscard]] double angle(const PathPoint& hinge, const PathPoint& other) const;
    QString to_string() const;
  };
  DEdge find_next_edge(const DEdge& dedge) const;

private:
  Graph m_graph;
  std::set<DEdge> m_edges;
  std::set<Face> m_faces;
};

}  // namespace omm
