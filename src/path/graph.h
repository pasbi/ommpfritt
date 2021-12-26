#pragma once

#include "geometry/point.h"
#include <deque>
#include <string>
#include <vector>

namespace omm
{

class PathPoint;
class Edge;
class Face;
class PathVector;

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

private:
  std::unique_ptr<Impl> m_impl;
};

}  // namespace omm
