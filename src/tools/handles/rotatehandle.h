#pragma once

#include "tools/handles/handle.h"
#include "tools/tool.h"
#include <QPainter>

namespace omm
{
template<typename ToolT> class RotateHandle : public Handle
{
public:
  RotateHandle(ToolT& tool) : Handle(tool)
  {
  }

  void draw(QPainter& painter) const override
  {
    painter.setTransform(tool.transformation().to_qtransform(), true);
    static const auto path = []() {
      QPainterPath path;
      path.addEllipse(-RADIUS, -RADIUS, 2 * RADIUS, 2 * RADIUS);
      return path;
    }();

    QPen pen;
    pen.setCosmetic(true);

    static constexpr double FILL_WIDTH = 8.0;
    static constexpr double OUTLINE_WIDTH = 2.0;

    pen.setColor(ui_color("rotate-ring-fill"));
    pen.setWidthF(FILL_WIDTH);
    painter.setPen(pen);
    painter.drawPath(path);

    pen.setColor(ui_color("rotate-ring-outline"));
    pen.setWidthF(OUTLINE_WIDTH);
    painter.setPen(pen);
    painter.drawPath(path);
  }

  [[nodiscard]] bool contains_global(const Vec2f& point) const override
  {
    const auto transformation = tool.transformation();
    const auto global_point = transformation.inverted().apply_to_position(point);
    const double r = global_point.euclidean_norm();
    return RADIUS - interact_epsilon() <= r && r <= RADIUS + interact_epsilon();
  }

  bool mouse_press(const Vec2f& pos, const QMouseEvent& e) override
  {
    m_transformation = tool.transformation();
    return Handle::mouse_press(pos, e);
  }

  bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e) override
  {
    Handle::mouse_move(delta, pos, e);
    if (status() == HandleStatus::Active) {
      const auto ti = m_transformation.inverted();
      const auto global_pos = ti.apply_to_position(pos);
      const auto origin = ti.apply_to_position(press_pos());

      double angle = global_pos.arg() - origin.arg();
      if (Tool::integer_transformation()) {
        static constexpr double step = 15 * M_PI_180;
        angle = step * static_cast<int>(angle / step);
      }

      const auto t = ObjectTransformation().rotated(angle).transformed(ti);
      static_cast<ToolT&>(tool).transform_objects(t);
      static_cast<ToolT&>(tool).tool_info = QString("%1°").arg(angle * M_180_PI);
      return true;
    } else {
      return false;
    }
  }

private:
  static constexpr double RADIUS = 100;
  ObjectTransformation m_transformation;
};

}  // namespace omm
