#pragma once

#include "geometry/direction.h"
#include <2geom/curve.h>
#include <QString>
#include <memory>

namespace omm
{

class Edge;
class PathPoint;

template<typename EdgePtr> struct DEdgeBase
{
  explicit DEdgeBase(EdgePtr edge, Direction direction);
  static DEdgeBase fwd(EdgePtr edge);
  static DEdgeBase bwd(EdgePtr edge);
  DEdgeBase() = default;
  EdgePtr edge = nullptr;
  [[nodiscard]] bool is_valid() const noexcept;
  Direction direction = Direction::Forward;
  template<typename OtherEdgePtr> [[nodiscard]] bool operator<(const DEdgeBase<OtherEdgePtr>& other) const
  {
    static constexpr auto to_tuple = [](const auto& d) { return std::tuple{d.edge, d.direction}; };
    return to_tuple(*this) < to_tuple(other);
  }

  template<typename OtherEdgePtr> [[nodiscard]] bool operator>(const DEdgeBase<OtherEdgePtr>& other) const
  {
    return other < *this;
  }

  template<typename OtherEdgePtr> [[nodiscard]] bool operator==(const DEdgeBase<OtherEdgePtr>& other) const
  {
    return edge == other.edge && direction == other.direction;
  }

  [[nodiscard]] PathPoint& end_point() const;
  [[nodiscard]] PathPoint& start_point() const;
  [[nodiscard]] double start_angle() const;
  [[nodiscard]] double end_angle() const;
  [[nodiscard]] std::unique_ptr<Geom::Curve> to_geom_curve() const;
  QString to_string() const;

private:
  [[nodiscard]] double angle(const Direction key_direction, const PathPoint& hinge, const PathPoint& other) const;
};

using DEdge = DEdgeBase<Edge*>;
using DEdgeConst = DEdgeBase<const Edge*>;

}  // namespace omm
