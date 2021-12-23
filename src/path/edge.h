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
  std::vector<PathPoint*> points() const;
  QString label() const;

  Point start_geometry() const;
  Point end_geometry() const;
  PathPoint* start_point() const;
  PathPoint* end_point() const;

  bool flipped = false;
  PathPoint* a = nullptr;
  PathPoint* b = nullptr;
};

}  // namespace omm
