#pragma once

#include "geometry/vec2.h"
#include "tools/handles/handle.h"
#include "tools/tool.h"

namespace omm
{
template<typename ToolT> class ScaleBandHandle : public Handle
{
public:
  ScaleBandHandle(ToolT& tool) : Handle(tool)
  {
  }

  [[nodiscard]] bool contains_global(const Vec2f& point) const override
  {
    const Vec2f global_point = tool.transformation().inverted().apply_to_position(point);
    const double x = global_point.x;
    const double y = global_point.y;
    return x + y < r + width / 2.0 && x + y > r - width / 2.0 && x > stop && y > stop;
  }

  void draw(QPainter& painter) const override
  {
    painter.setPen(ui_color("band"));
    painter.setBrush(ui_color("band fill"));
    painter.setTransform(tool.transformation().to_qtransform(), true);
    static constexpr std::array<QPointF, 5> polyline = {QPointF(stop, r - width / 2.0 - stop),
                                                        QPointF(stop, r + width / 2.0 - stop),
                                                        QPointF(r + width / 2.0 - stop, stop),
                                                        QPointF(r - width / 2.0 - stop, stop),
                                                        QPointF(stop, r - width / 2.0 - stop)};
    painter.drawPolygon(polyline.data(), polyline.size());
  }

  bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e) override
  {
    Handle::mouse_move(delta, pos, e);
    if (status() == HandleStatus::Active) {
      static constexpr double STEP = 0.1;
      const auto ti = tool.transformation().inverted();
      const auto global_pos = ti.apply_to_position(pos);
      const auto origin = ti.apply_to_position(press_pos());

      double s = global_pos.euclidean_norm() / origin.euclidean_norm();
      s = discretize(s, STEP);

      if (constexpr auto eps = 10e-10; std::abs(s) < eps) {
        s = std::copysign(eps, s);
      }

      auto t = omm::ObjectTransformation().scaled(Vec2f(s, s));
      t = t.transformed(ti);
      static_cast<ToolT&>(tool).transform_objects(t);
      static_cast<ToolT&>(tool).tool_info = QString("%1").arg(s);
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
