#pragma once

#include <QString>
#include <vector>

namespace omm
{

class PathPoint;
class Point;

class Edge
{
public:
  Edge() = default;
  [[nodiscard]] QString label() const;

  [[nodiscard]] Point start_geometry() const;
  [[nodiscard]] Point end_geometry() const;
  [[nodiscard]] PathPoint* start_point() const;
  [[nodiscard]] PathPoint* end_point() const;

  bool flipped = false;
  PathPoint* a = nullptr;
  PathPoint* b = nullptr;

  // Edge equality is not unabiguously implementable.
  // It's clear that numerical coincidence should not matter (1).
  // Also, direction should not matter, because we're dealing with undirected graphs (2).
  // It'd be also a good idea to distinguish joined points (two edges between A and B are not equal)
  // because tangents can make these edges appear very different (3).
  // Usually, multiple edges only occur between joined points and can be distinguished well.
  // However, consider the loop (A) --e1-- (B) --e2-- (A):
  // e1 and e2 are not distinguishable when ignoring direction, no joined points are involved to
  // distinguish.
  // That is, requirement (2) and (3) conflict.
  // In practice that is no problem because the equality operator is not required.
  friend bool operator==(const Edge&, const Edge&) = delete;
};

}  // namespace omm
