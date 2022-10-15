#pragma once

#include "graph.h"
#include "path/dedge.h"
#include <list>
#include <map>
#include <set>

namespace omm
{

class Face;

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
