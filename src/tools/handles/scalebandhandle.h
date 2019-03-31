#pragma once

#include <armadillo>
#include "tools/handles/handle.h"
#include "geometry/util.h"
#include "renderers/abstractrenderer.h"

namespace omm
{

template<typename ToolT>
class ScaleBandHandle : public Handle
{
public:
  ScaleBandHandle(ToolT& tool) : Handle(tool, true)
  {
    set_style(Status::Active, omm::SolidStyle(omm::Color(1.0, 1.0, 1.0)));
    set_style(Status::Hovered, omm::SolidStyle(omm::Color(0.7, 0.7, 0.7)));
    set_style(Status::Inactive, omm::SolidStyle(omm::Color(0.5, 0.5, 0.5)));
  }

  bool contains_global(const arma::vec2& point) const override
  {
    const arma::vec2 global_point = transformation().inverted().apply_to_position(point);
    const double x = global_point(0);
    const double y = global_point(1);
    return x+y < r + width/2.0 && x+y > r - width/2.0 && arma::all(global_point > stop);
  }

  void draw(omm::AbstractRenderer& renderer) const override
  {
    const auto path = std::vector { Point(arma::vec2{ stop, r - width/2.0 - stop }),
                                    Point(arma::vec2{ stop, r + width/2.0 - stop }),
                                    Point(arma::vec2{ r + width/2.0 - stop, stop }),
                                    Point(arma::vec2{ r - width/2.0 - stop, stop }) };
    renderer.draw_spline(path, current_style(), true);
  }

  bool mouse_move(const arma::vec2& delta, const arma::vec2& pos, const QMouseEvent& e) override
  {
    Handle::mouse_move(delta, pos, e);
    if (status() == Status::Active) {
      const auto ti = transformation().inverted();
      const auto global_pos = ti.apply_to_position(pos);
      const auto origin = ti.apply_to_position(press_pos());
      const auto delta = global_pos - origin;
      double s = arma::norm(global_pos) / arma::norm(global_pos - delta);
      if (tool.integer_transformation()) {
        static constexpr auto step = 0.1;
        if (std::abs(s) > step) { // s must never be zero.
          s = step * static_cast<int>(s / step);
        }
      }
      if (constexpr auto eps = 10e-10; std::abs(s) < eps) { s = std::copysign(eps, s); }

      const auto t = omm::ObjectTransformation().scaled(arma::vec2{ s, s });
      static_cast<ToolT&>(tool).transform_objects_absolute(t, true);
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
