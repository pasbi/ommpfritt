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
    m_bounding_box = static_cast<const ToolT&>(tool).bounding_box();
    m_left_fringe = std::abs(m_bounding_box.left() - point.x) < eps;
    m_right_fringe = std::abs(m_bounding_box.right() - point.x) < eps;
    m_top_fringe = std::abs(m_bounding_box.top() - point.y) < eps;
    m_bottom_fringe = std::abs(m_bounding_box.bottom() - point.y) < eps;
    return m_left_fringe || m_right_fringe || m_top_fringe || m_bottom_fringe;
  }

  bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e) override
  {
    Handle::mouse_move(delta, pos, e);
    if (status() != Status::Active) {
      return false;
    }

    const auto ti = transformation().inverted();
    const auto global_pos = ti.apply_to_position(pos);
    const auto bounding_box = ti.apply(m_bounding_box);
    const auto center = ti.apply_to_position(Vec2f::o());

    const double f = 2;
    Vec2f s(1.0, 1.0);
    if (m_left_fringe) {
      s.x += f * (global_pos.x - bounding_box.left()) / (bounding_box.left() - center.x);
    } else if (m_right_fringe) {
      s.x -= f * (global_pos.x - bounding_box.right()) / (bounding_box.right() - center.x);
    }
    s.x = 1.0 / s.x;
    if (m_top_fringe) {
      s.y -= f * (global_pos.y - bounding_box.top()) / (global_pos.y - center.y);
    } else if (m_bottom_fringe) {
      s.y += f * (global_pos.y - bounding_box.bottom()) / (global_pos.y - center.y);
    }
    const auto t = omm::ObjectTransformation().scaled(s);
    static_cast<ToolT&>(tool).transform_objects_absolute(t, true);
    static_cast<ToolT&>(tool).tool_info = QString("%1, %2").arg(s.x).arg(s.y).toStdString();
    return true;
  }

  void draw(Painter &renderer) const override
  {
    const BoundingBox bounding_box = static_cast<const ToolT&>(tool).bounding_box();
    renderer.set_style(current_style());
    renderer.painter->drawRect(bounding_box);
  }
  mutable BoundingBox m_bounding_box;
  mutable bool m_left_fringe = false;
  mutable bool m_right_fringe = false;
  mutable bool m_top_fringe = false;
  mutable bool m_bottom_fringe = false;

};

}  // namespace omm
