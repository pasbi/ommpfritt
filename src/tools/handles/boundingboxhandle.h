#pragma once

#include "tools/handles/handle.h"
#include "tools/tool.h"
#include "renderers/painter.h"

namespace omm
{

template<typename ToolT>
class BoundingBoxHandle : public Handle
{
public:
  explicit BoundingBoxHandle(ToolT& tool) : Handle(tool, false)
  {
    set_style(Status::Active, omm::ContourStyle(omm::Color(1.0, 1.0, 1.0)));
    set_style(Status::Hovered, omm::ContourStyle(omm::Color(0.7, 0.7, 0.7)));
    set_style(Status::Inactive, omm::ContourStyle(omm::Color(0.5, 0.5, 0.5)));
  }

  bool contains_global(const Vec2f& point) const override
  {
    static constexpr double eps = 4;
    const BoundingBox bounding_box = static_cast<const ToolT&>(tool).bounding_box();
    const bool left_fringe = std::abs(bounding_box.left() - point.x) < eps;
    const bool right_fringe = std::abs(bounding_box.right() - point.x) < eps;
    const bool top_fringe = std::abs(bounding_box.top() - point.y) < eps;
    const bool bottom_fringe = std::abs(bounding_box.bottom() - point.y) < eps;
    return left_fringe || right_fringe || top_fringe || bottom_fringe;
  }

  bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e) override
  {
    Handle::mouse_move(delta, pos, e);
    if (status() != Status::Active) {
      return false;
    }

    const auto ti = transformation().inverted();
    const auto global_pos = ti.apply_to_position(pos);
    const auto origin = ti.apply_to_position(press_pos());
    const auto delta_ = global_pos - origin;
    double s = global_pos.euclidean_norm() / (global_pos - delta_).euclidean_norm();
    if (tool.integer_transformation()) {
      static constexpr auto step = 0.1;
      if (std::abs(s) > step) { // s must never be zero.
        s = step * static_cast<int>(s / step);
      }
    }
    if (constexpr auto eps = 10e-10; std::abs(s) < eps) { s = std::copysign(eps, s); }

    const auto t = omm::ObjectTransformation().scaled(Vec2f(s, s));
    static_cast<ToolT&>(tool).transform_objects_absolute(t, true);
    static_cast<ToolT&>(tool).tool_info = QString("%1").arg(s).toStdString();
    return true;
  }

  void draw(Painter &renderer) const override
  {
    const BoundingBox bounding_box = static_cast<const ToolT&>(tool).bounding_box();
    renderer.set_style(current_style());
    renderer.painter->drawRect(bounding_box);
  }

};

}  // namespace omm
