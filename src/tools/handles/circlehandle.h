#pragma once

#include "tools/handles/handle.h"

namespace omm
{

class CircleHandle : public Handle
{
public:
  using Handle::Handle;
  void draw(AbstractRenderer& renderer) const override;
  bool contains(const arma::vec2& point) const override;
  void set_radius(double r);

  static constexpr double RADIUS = 100;
private:
  double m_radius;
};

template<typename ToolT>
class RotateHandle : public CircleHandle
{
public:
  RotateHandle(ToolT& tool) : m_tool(tool) { }

  bool mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover) override
  {
    Handle::mouse_move(delta, pos, allow_hover);
    if (status() == Status::Active) {
      const arma::vec2 origin = pos - delta;
      double origin_angle = atan2(origin(1), origin(0));
      double pos_angle = atan2(pos(1), pos(0));
      m_tool.transform_objects(omm::ObjectTransformation().rotated(pos_angle - origin_angle));
      return true;
    } else {
      return false;
    }
  }

private:
  ToolT& m_tool;
};

}  // namespace omm
