#pragma once

#include "common.h"
#include "renderers/painter.h"
#include "tools/handles/handle.h"
#include "tools/tool.h"
#include <QMouseEvent>

namespace omm
{
class AbstractBoundingBoxHandle : public Handle
{
protected:
  using Handle::Handle;
  enum class Fringe {
    None = 0x0,
    Left = 0x1,
    Bottom = 0x2,
    Right = 0x4,
    Top = 0x8,
    Horizontal = Left | Right,
    Vertical = Bottom | Top
  };
};

}  // namespace omm

template<>
struct omm::EnableBitMaskOperators<omm::AbstractBoundingBoxHandle::Fringe> : std::true_type {
};

namespace omm
{
template<typename ToolT> class BoundingBoxHandle : public AbstractBoundingBoxHandle
{
public:
  explicit BoundingBoxHandle(ToolT& tool) : AbstractBoundingBoxHandle(tool)
  {
    m_bounding_box = static_cast<const ToolT&>(tool).bounding_box();
  }

  bool mouse_press(const Vec2f& pos, const QMouseEvent& e) override
  {
    m_bounding_box = static_cast<const ToolT&>(tool).bounding_box();
    m_tool_origin = tool.transformation().apply_to_position(Vec2f::o());
    m_symmetric = tool.property(ToolT::SYMMETRIC_PROPERTY_KEY)->template value<bool>();
    m_active_fringe = get_fringe(pos);
    return Handle::mouse_press(pos, e);
  }

  bool contains_global(const Vec2f& point) const override
  {
    return get_fringe(point) != Fringe::None;
  }

  bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e) override
  {
    Q_UNUSED(delta)
    Handle::mouse_move(delta, pos, e);
    if (status() != HandleStatus::Active) {
      return false;
    }
    auto& tool = static_cast<ToolT&>(this->tool);
    const bool discrete = tool.integer_transformation();
    const bool isotropic = (e.modifiers() & Qt::ControlModifier) != 0u;

    Vec2f anchor;
    if (m_symmetric) {
      anchor = m_tool_origin;
    } else {
      if (!!(m_active_fringe & Fringe::Left)) {
        anchor.x = m_bounding_box.right();
      } else if (!!(m_active_fringe & Fringe::Right)) {
        anchor.x = m_bounding_box.left();
      } else {
        anchor.x = (m_bounding_box.left() + m_bounding_box.right()) / 2.0;
      }
      if (!!(m_active_fringe & Fringe::Top)) {
        anchor.y = m_bounding_box.bottom();
      } else if (!!(m_active_fringe & Fringe::Bottom)) {
        anchor.y = m_bounding_box.top();
      } else {
        anchor.y = (m_bounding_box.top() + m_bounding_box.bottom()) / 2.0;
      }
    }

    Vec2f s = (anchor - pos) / (anchor - press_pos());

    if (!(m_active_fringe & Fringe::Horizontal)) {
      s.x = 1.0;
    }
    if (!(m_active_fringe & Fringe::Vertical)) {
      s.y = 1.0;
    }

    if (isotropic) {
      double n = [s]() {
        if (s.x == 1.0) {
          return s.y;
        } else if (s.y == 1.0) {
          return s.x;
        } else {
          return std::max(std::abs(s.x), std::abs(s.y));
        }
      }();
      for (std::size_t i : {0u, 1u}) {
        s[i] = std::copysign(n, s[i]);
      }
    }

    if (discrete) {
      for (const std::size_t i : {0u, 1u}) {
        static constexpr double step = 0.1;
        s[i] = step * static_cast<int>(s[i] / step);
      }
    }

    ObjectTransformation transformation;
    transformation = transformation.apply(ObjectTransformation().translated(anchor));
    transformation = transformation.apply(ObjectTransformation().scaled(s));
    transformation = transformation.apply(ObjectTransformation().translated(-anchor));
    tool.tool_info = QString("%1, %2").arg(s.x).arg(s.y);
    tool.transform_objects(transformation);
    return true;
  }

  void draw(QPainter& painter) const override
  {
    const BoundingBox bounding_box = static_cast<const ToolT&>(tool).bounding_box();
    painter.setPen(ui_color("bounding-box"));
    painter.drawRect(bounding_box);
  }

  Fringe get_fringe(const Vec2f& pos) const
  {
    static constexpr double eps = 4;
    Fringe f = Fringe::None;
    if (std::abs(m_bounding_box.left() - pos.x) < eps) {
      f |= Fringe::Left;
    }
    if (std::abs(m_bounding_box.right() - pos.x) < eps) {
      f |= Fringe::Right;
    }
    if (std::abs(m_bounding_box.top() - pos.y) < eps) {
      f |= Fringe::Top;
    }
    if (std::abs(m_bounding_box.bottom() - pos.y) < eps) {
      f |= Fringe::Bottom;
    }

    if ((!!(f & Fringe::Bottom) && !!(f & Fringe::Top))
        || (!!(f & Fringe::Right) && !!(f & Fringe::Left))) {
      // It's only possible to select left and right or top and bottom at the same time if the
      // bounding box is smaller than epsilon. In this case it's very very unlikely that anything
      // sane happens in response to the event. Hence, behave as if nothing was selected.
      f = Fringe::None;
    }
    return f;
  }

  BoundingBox m_bounding_box;
  bool m_symmetric = false;
  mutable Vec2f m_tool_origin = Vec2f::o();
  Fringe m_active_fringe{};
};

}  // namespace omm
