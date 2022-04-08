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
  explicit Edge(PathPoint& a, PathPoint& b);
  [[nodiscard]] QString label() const;

  [[nodiscard]] Point start_geometry() const;
  [[nodiscard]] Point end_geometry() const;
  [[nodiscard]] PathPoint* start_point() const;
  [[nodiscard]] PathPoint* end_point() const;

  bool flipped = false;
  PathPoint* a = nullptr;
  PathPoint* b = nullptr;


};

}  // namespace omm
