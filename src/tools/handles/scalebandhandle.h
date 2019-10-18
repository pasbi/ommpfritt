#pragma once

#include "geometry/vec2.h"
#include "tools/handles/handle.h"
#include "geometry/util.h"
#include "renderers/painter.h"
#include "tools/tool.h"

namespace omm
{

template<typename ToolT>
class ScaleBandHandle : public Handle
{
public:
  ScaleBandHandle(ToolT& tool) : Handle(tool)
  {
    set_style(Status::Active, SolidStyle(Colors::WHITE));
    set_style(Status::Hovered, SolidStyle(Color(Color::Model::RGBA, { 0.7, 0.7, 0.7, 1.0 })));
    set_style(Status::Inactive, SolidStyle(Color(Color::Model::RGBA, { 0.5, 0.5, 0.5 })));
  }

  bool contains_global(const Vec2f& point) const override
  {
    const Vec2f global_point = tool.transformation().inverted().apply_to_position(point);
    const double x = global_point.x;
    const double y = global_point.y;
    return x+y < r + width/2.0 && x+y > r - width/2.0 && x > stop && y > stop;
  }

  void draw(Painter& painter) const override
  {
    painter.push_transformation(tool.transformation());
    QPointF polyline[] = { QPointF(stop, r - width/2.0 - stop),
                           QPointF(stop, r + width/2.0 - stop),
                           QPointF(r + width/2.0 - stop, stop),
                           QPointF(r - width/2.0 - stop, stop),
                           QPointF(stop, r - width/2.0 - stop) };
    painter.set_style(current_style());
    painter.painter->drawPolygon(polyline, 5);
    painter.pop_transformation();
  }

  bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e) override
  {
    Handle::mouse_move(delta, pos, e);
    if (status() == Status::Active) {
      const auto ti = tool.transformation().inverted();
      const auto global_pos = ti.apply_to_position(pos);
      const auto origin = ti.apply_to_position(press_pos());
      const auto delta = global_pos - origin;
      double s = global_pos.euclidean_norm() / (global_pos - delta).euclidean_norm();
      if (tool.integer_transformation()) {
        static constexpr auto step = 0.1;
        if (std::abs(s) > step) { // s must never be zero.
          s = step * static_cast<int>(s / step);
        }
      }
      if (constexpr auto eps = 10e-10; std::abs(s) < eps) { s = std::copysign(eps, s); }

      {
        auto t = omm::ObjectTransformation().scaled(Vec2f(s, s));
        t = t.transformed(ti);
        static_cast<ToolT&>(tool).transform_objects(t);
      }
      static_cast<ToolT&>(tool).tool_info = QString("%1").arg(s).toStdString();
      return true;
    } else {
      return false;
    }
  }

private:
  static constexpr double width = 40.0;
  static constexpr double r = 70.0;
  static constexpr double stop = 10.0;
};

}  // namespace omm
