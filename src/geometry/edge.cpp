#include "edge.h"

namespace omm
{

Edge::Edge(const Vec2f& a, const Vec2f& b) : a(a), b(b) { }
Edge::Edge() {}

double Edge::intersect(const Edge &other) const
{
  const auto& x1 = a.x;
  const auto& y1 = a.y;
  const auto& x2 = b.x;
  const auto& y2 = b.y;
  const auto& x3 = other.a.x;
  const auto& y3 = other.a.y;
  const auto& x4 = other.b.x;
  const auto& y4 = other.b.y;
  const double denom = (x1-x3)*(y3-y4)-(y1-y3)*(x3-x4);
  const double nom   = (x1-x2)*(y3-y4)-(y1-y2)*(x3-x4);
  return denom / nom;
}

bool Edge::intersects(const Edge &e1, const Edge &e2, const double eps)
{
  double t1 = e1.intersect(e2);
  double t2 = e2.intersect(e1);
  return t1 > eps && t2 > eps && t1 < 1-eps && t2 < 1-eps;
}

std::ostream &operator<<(std::ostream &stream, const Edge &edge)
{
  stream << "Edge[" << edge.a << ", " << edge.b << "]";
  return stream;
}

}  // namespace omm
