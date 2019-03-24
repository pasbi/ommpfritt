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
  ScaleBandHandle(ToolT& tool) : Handle(tool, true), m_tool(tool)
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
      const auto ti = m_tool.transformation().inverted();
      arma::vec2 gpos = ti.apply_to_position(pos);
      arma::vec2 gdelta = ti.apply_to_direction(delta);
      const auto t = omm::ObjectTransformation().scaled(gpos / (gpos - gdelta));
      m_tool.transform_objects(t, true);
      return true;
    } else {
      return false;
    }
  }

private:
  ToolT& m_tool;
  static constexpr double width = 40.0;
  static constexpr double r = 70.0;
  static constexpr double stop = 10.0;
};

}  // namespace omm
