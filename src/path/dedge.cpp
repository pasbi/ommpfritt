#include "path/dedge.h"
#include "geometry/point.h"
#include "path/edge.h"
#include "path/pathpoint.h"

#include <2geom/bezier-curve.h>

namespace omm
{

template<typename EdgePtr> DEdgeBase<EdgePtr>::DEdgeBase(EdgePtr const edge, const Direction direction)
  : edge(edge), direction(direction)
{
}

template<typename EdgePtr> DEdgeBase<EdgePtr> DEdgeBase<EdgePtr>::fwd(EdgePtr edge)
{
  return DEdgeBase{edge, Direction::Forward};
}

template<typename EdgePtr> DEdgeBase<EdgePtr> DEdgeBase<EdgePtr>::bwd(EdgePtr edge)
{
  return DEdgeBase{edge, Direction::Backward};
}

template<typename EdgePtr> PathPoint& DEdgeBase<EdgePtr>::end_point() const
{
  return *edge->end_point(direction);
}

template<typename EdgePtr> PathPoint& DEdgeBase<EdgePtr>::start_point() const
{
  return *edge->start_point(direction);
}

template<typename EdgePtr> double DEdgeBase<EdgePtr>::start_angle() const
{
  return angle(Direction::Forward, start_point(), end_point());
}

template<typename EdgePtr> double DEdgeBase<EdgePtr>::end_angle() const
{
  return angle(Direction::Backward, end_point(), start_point());
}

template<typename EdgePtr> std::unique_ptr<Geom::Curve> DEdgeBase<EdgePtr>::to_geom_curve() const
{
  const std::vector<Vec2f> pts{
      start_point().geometry().position(), start_point().geometry().tangent_position({edge->path(), direction}),
      end_point().geometry().tangent_position({edge->path(), other(direction)}), end_point().geometry().position()};
  return std::make_unique<Geom::BezierCurveN<3>>(util::transform(std::move(pts), &Vec2f::to_geom_point));
}

template<typename EdgePtr> double DEdgeBase<EdgePtr>::angle(const Direction key_direction, const PathPoint& hinge,
                                                            const PathPoint& other_point) const
{
  const auto key = Point::TangentKey{edge->path(), key_direction};
  const auto tangent = hinge.geometry().tangent(key);
  static constexpr double eps = 0.001;
  if (tangent.magnitude > eps) {
    return tangent.argument;
  } else {
    const auto other_key = Point::TangentKey{edge->path(), other(key_direction)};
    const auto t_pos = other_point.geometry().tangent_position(other_key);
    const auto o_pos = hinge.geometry().position();
    return PolarCoordinates(t_pos - o_pos).argument;
  }
}

template<typename EdgePtr> QString DEdgeBase<EdgePtr>::to_string() const
{
  if (edge == nullptr) {
    return "null";
  }
  return (direction == Direction::Forward ? "" : "r") + edge->label();
}

template struct DEdgeBase<Edge*>;
template struct DEdgeBase<const Edge*>;

}  // namespace omm
