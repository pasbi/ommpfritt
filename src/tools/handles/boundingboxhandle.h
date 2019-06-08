#pragma once

#include "tools/handles/handle.h"
#include "tools/tool.h"
#include "renderers/painter.h"
#include <QMouseEvent>

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

    const auto ti = tool.transformation().inverted();
    const auto global_pos = ti.apply_to_position(pos);
    const auto origin = ti.apply_to_position(press_pos());
    const auto delta_ = global_pos - origin;
    Vec2f s = (global_pos - delta_) / global_pos;

    if (tool.integer_transformation()) {
      for (const std::size_t i : { 0u, 1u }) {
        double step = 0.1;
        if (std::abs(s[i]) > step) {
          s[i] = step * static_cast<int>(s[i] / step);
        } else {
          double step = 0.01;
          s[i] = step * static_cast<int>(s[i] / step);
        }
      }
    }
    if (!m_left_fringe && !m_right_fringe) {
      s.x = 1.0;
    }
    if (!m_bottom_fringe && !m_top_fringe) {
      s.y = 1.0;
    }

    if (e.modifiers() & Qt::ControlModifier) {
      double n;
      if (s.x == 1.0) {
        n = s.y;
      } else if (s.y == 1.0) {
        n = s.x;
      } else {
        n = std::max(std::abs(s.x), std::abs(s.y));
      }
      for (std::size_t i : { 0u, 1u }) {
        s[i] = std::copysign(n, s[i]);
      }
    }

    for (const std::size_t i : { 0u, 1u }) {
      if (static constexpr auto eps = 0.01; std::abs(s[i]) < eps) {
        s[i] = std::copysign(eps, s[i]);
      }
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
