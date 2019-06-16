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
  explicit BoundingBoxHandle(ToolT& tool) : Handle(tool)
  {
    set_style(Status::Active, omm::ContourStyle(omm::Color(1.0, 1.0, 1.0)));
    set_style(Status::Hovered, omm::ContourStyle(omm::Color(0.7, 0.7, 0.7)));
    set_style(Status::Inactive, omm::ContourStyle(omm::Color(0.5, 0.5, 0.5)));
  }

  bool mouse_press(const Vec2f& pos, const QMouseEvent& e, bool force) override
  {
    static constexpr double eps = 4;
    m_bounding_box = static_cast<const ToolT&>(tool).bounding_box();
    m_left_fringe = std::abs(m_bounding_box.left() - pos.x) < eps;
    m_right_fringe = std::abs(m_bounding_box.right() - pos.x) < eps;
    m_top_fringe = std::abs(m_bounding_box.top() - pos.y) < eps;
    m_bottom_fringe = std::abs(m_bounding_box.bottom() - pos.y) < eps;
    m_tool_origin = tool.transformation().apply_to_position(Vec2f::o());
    m_symmetric = tool.property(ToolT::SYMMETRIC_PROPERTY_KEY)->template value<bool>();
    return Handle::mouse_press(pos, e, force);
  }

  bool contains_global(const Vec2f& point) const override
  {
    Q_UNUSED(point)
    return m_left_fringe || m_right_fringe || m_top_fringe || m_bottom_fringe;
  }

  bool mouse_move(const Vec2f &delta, const Vec2f &pos, const QMouseEvent &e) override
  {
    Q_UNUSED(delta)
    if (status() != Status::Active) {
      return false;
    }

    ToolT& tool = static_cast<ToolT&>(this->tool);
    const bool discrete = tool.integer_transformation();
    const bool isotropic = e.modifiers() & Qt::ControlModifier;

    Vec2f anchor;
    if (m_symmetric) {
      anchor = m_tool_origin;
    } else {
      if (m_left_fringe) {
        anchor.x = m_bounding_box.right();
      } else if (m_right_fringe) {
        anchor.x = m_bounding_box.left();
      } else {
        anchor.x = (m_bounding_box.left() + m_bounding_box.right()) / 2.0;
      }
      if (m_top_fringe) {
        anchor.y = m_bounding_box.bottom();
      } else if (m_bottom_fringe) {
        anchor.y = m_bounding_box.top();
      } else {
        anchor.y = (m_bounding_box.top() + m_bounding_box.bottom()) / 2.0;
      }
    }

    Vec2f s = (anchor - pos) / (anchor - press_pos());

    if (!m_left_fringe && !m_right_fringe) {
      s.x = 1.0;
    }
    if (!m_top_fringe && !m_bottom_fringe) {
      s.y = 1.0;
    }

    if (isotropic) {
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

    if (discrete) {
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

    ObjectTransformation transformation;
    transformation = transformation.apply(ObjectTransformation().translated(anchor));
    transformation = transformation.apply(ObjectTransformation().scaled(s));
    transformation = transformation.apply(ObjectTransformation().translated(-anchor));
    tool.tool_info = QString("%1, %2").arg(s.x).arg(s.y).toStdString();
    tool.transform_objects_absolute(transformation, false);
    return true;
  }

  void draw(Painter &renderer) const override
  {
    const BoundingBox bounding_box = static_cast<const ToolT&>(tool).bounding_box();
    renderer.set_style(current_style());
    renderer.painter->drawRect(bounding_box);
  }

  BoundingBox m_bounding_box;
  bool m_left_fringe = false;
  bool m_right_fringe = false;
  bool m_top_fringe = false;
  bool m_bottom_fringe = false;
  bool m_symmetric = false;
  mutable Vec2f m_tool_origin = Vec2f::o();

};

}  // namespace omm
