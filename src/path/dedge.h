#pragma once

#include "geometry/direction.h"

#include "edge.h"
namespace omm
{

struct DEdge
{
  explicit DEdge(Edge* edge, Direction direction);
  static DEdge fwd(Edge* edge);
  static DEdge bwd(Edge* edge);
  DEdge() = default;
  Edge* edge = nullptr;
  Direction direction = Direction::Forward;
  [[nodiscard]] bool operator<(const DEdge& other) const;
  [[nodiscard]] bool operator>(const DEdge& other) const;
  [[nodiscard]] bool operator==(const DEdge& other) const;
  [[nodiscard]] PathPoint& end_point() const;
  [[nodiscard]] PathPoint& start_point() const;
  [[nodiscard]] double start_angle() const;
  [[nodiscard]] double end_angle() const;
  [[nodiscard]] double angle(const PathPoint& hinge, const PathPoint& other) const;
  QString to_string() const;
};

}  // namespace omm
