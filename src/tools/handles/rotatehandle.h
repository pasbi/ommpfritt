#pragma once

#include <QPainter>
#include "renderers/painter.h"
#include "tools/handles/handle.h"
#include "tools/tool.h"

namespace omm
{

template<typename ToolT>
class RotateHandle : public Handle
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
      path.addEllipse(-RADIUS, -RADIUS, 2*RADIUS, 2*RADIUS);
      return path;
    }();

    QPen pen;
    pen.setCosmetic(true);

    pen.setColor(ui_color("rotate-ring-fill"));
    pen.setWidthF(8.0);
    painter.setPen(pen);
    painter.drawPath(path);

    pen.setColor(ui_color("rotate-ring-outline"));
    pen.setWidthF(2.0);
    painter.setPen(pen);
    painter.drawPath(path);
  }

  bool contains_global(const Vec2f& point) const override
  {
    const auto transformation = tool.transformation();
    const auto global_point = transformation.inverted().apply_to_position(point);
    const double r = global_point.euclidean_norm();
    return RADIUS - interact_epsilon() <= r && r <= RADIUS + interact_epsilon();
  }

  bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e) override
  {
    Handle::mouse_move(delta, pos, e);
    if (status() == HandleStatus::Active) {
      const auto transformation = tool.transformation();
      const auto global_pos = transformation.inverted().apply_to_position(pos);
      const auto origin = transformation.inverted().apply_to_position(press_pos());

      double angle = global_pos.arg() - origin.arg();
      if (tool.integer_transformation()) {
        static constexpr double step = 15 * M_PI / 180.0;
        angle = step * static_cast<int>(angle / step);
      }

      const auto inv_tool_transformation = tool.transformation().inverted();
      const auto t = ObjectTransformation().rotated(angle).transformed(inv_tool_transformation);
      static_cast<ToolT&>(tool).transform_objects(t);
      static_cast<ToolT&>(tool).tool_info = QString("%1°").arg(angle / M_PI * 180.0);
      return true;
    } else {
      return false;
    }
  }

private:
  static constexpr double RADIUS = 100;
};

}  // namespace omm
