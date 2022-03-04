#pragma once

#include "geometry/point.h"
#include <deque>
#include <string>
#include <vector>
#include <memory>

namespace omm
{

class PathPoint;
class Edge;
class Face;
class PathVector;  // NOLINT(bugprone-forward-declaration-namespace)

class Graph
{
public:
  class Impl;
  Graph(const PathVector& path_vector);
  Graph(const Graph& other) = delete;
  Graph(Graph&& other) = default;
  Graph& operator=(const Graph& other) = delete;
  Graph& operator=(Graph&& other) = default;
  ~Graph();

  [[nodiscard]] std::vector<Face> compute_faces() const;
  [[nodiscard]] QString to_dot() const;

  /**
   * @brief remove_articulation_edges an edge is articulated if it connects two articulated points,
   *  two points of degree one or less or an articulated point with a point of degree one or less.
   *  Removing articulated edges from a graph increase the number of components by one.
   *  Articulated edges can never be part of a face.
   */
  void remove_articulation_edges() const;

private:
  std::unique_ptr<Impl> m_impl;
};

}  // namespace omm
