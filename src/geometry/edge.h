#pragma once

#include "geometry/vec2.h"

namespace omm
{

class Edge
{
public:
  Edge(const Vec2f &a, const Vec2f &b);
  Edge();

  Vec2f a;
  Vec2f b;

  double intersect(const Edge& other) const;
  static bool intersects(const Edge& e1, const Edge& e2, const double eps);
};

std::ostream& operator<<(std::ostream& stream, const Edge& edge);

}  // namespace omm
