#include "path/dedge.h"
#include "geometry/point.h"
#include "path/pathpoint.h"

namespace omm
{

DEdge::DEdge(Edge* const edge, const Direction direction) : edge(edge), direction(direction)
{
}

DEdge DEdge::fwd(Edge* edge)
{
  return DEdge{edge, Direction::Forward};
}

DEdge DEdge::bwd(Edge* edge)
{
  return DEdge{edge, Direction::Backward};
}

bool DEdge::operator<(const DEdge& other) const
{
  static constexpr auto to_tuple = [](const auto& d) { return std::tuple{d.edge, d.direction}; };
  return to_tuple(*this) < to_tuple(other);
}

bool DEdge::operator>(const DEdge& other) const
{
  return other < *this;
}

bool DEdge::operator==(const DEdge& other) const
{
  return edge == other.edge && direction == other.direction;
}

PathPoint& DEdge::end_point() const
{
  return *edge->end_point(direction);
}

PathPoint& DEdge::start_point() const
{
  return *edge->start_point(direction);
}

double DEdge::start_angle() const
{
  return angle(start_point(), end_point());
}

double DEdge::end_angle() const
{
  return angle(end_point(), start_point());
}

double DEdge::angle(const PathPoint& hinge, const PathPoint& other_point) const
{
  const auto key = Point::TangentKey{edge->path(), direction};
  const auto tangent = hinge.geometry().tangent(key);
  static constexpr double eps = 0.1;
  if (tangent.magnitude > eps) {
    return tangent.argument;
  } else {
    const auto other_key = Point::TangentKey{edge->path(), other(direction)};
    const auto t_pos = other_point.geometry().tangent_position(other_key);
    const auto o_pos = hinge.geometry().position();
    return PolarCoordinates(t_pos - o_pos).argument;
  }
}

QString DEdge::to_string() const
{
  if (edge == nullptr) {
    return "null";
  }
  return (direction == Direction::Forward ? "" : "r") + edge->label();
}

}  // namespace omm
