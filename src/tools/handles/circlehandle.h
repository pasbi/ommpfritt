#pragma once

#include "tools/handles/handle.h"

namespace omm
{

class CircleHandle : public Handle
{
public:
  explicit CircleHandle(Tool& tool);
  void draw(AbstractRenderer& renderer) const override;
  bool contains_global(const arma::vec2& point) const override;
  void set_radius(double r);

  static constexpr double RADIUS = 100;
private:
  double m_radius;
};

template<typename ToolT>
class RotateHandle : public CircleHandle
{
public:
  RotateHandle(ToolT& tool) : CircleHandle(tool), m_tool(tool) { }

  bool mouse_move(const arma::vec2& delta, const arma::vec2& pos, const QMouseEvent& e) override
  {
    Handle::mouse_move(delta, pos, e);
    if (status() == Status::Active) {
      const auto global_pos = transformation().inverted().apply_to_position(pos);
      const auto global_delta = transformation().inverted().apply_to_direction(delta);
      const arma::vec2 origin = global_pos - global_delta;
      double origin_angle = atan2(origin(1), origin(0));
      double pos_angle = atan2(global_pos(1), global_pos(0));
      const auto t = omm::ObjectTransformation().rotated(pos_angle - origin_angle);
      m_tool.transform_objects(t, transform_in_tool_space);
      return true;
    } else {
      return false;
    }
  }

private:
  ToolT& m_tool;
};

}  // namespace omm
