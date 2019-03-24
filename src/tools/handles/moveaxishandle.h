#pragma once

#include <armadillo>
#include "tools/handles/handle.h"
#include "renderers/abstractrenderer.h"
#include "geometry/util.h"
#include <QDebug>

namespace omm
{

enum class MoveAxisHandleDirection { X, Y };

template<typename ToolT, MoveAxisHandleDirection direction>
class MoveAxisHandle : public Handle
{
public:
  MoveAxisHandle(ToolT& tool)
    : Handle(tool, true)
    , m_tool(tool)
    , m_direction(direction == MoveAxisHandleDirection::X ? arma::vec2{100.0, 0.0}
                                                          : arma::vec2{0.0, 100.0})
  {
    switch (direction) {
    case MoveAxisHandleDirection::X:
      set_style(Status::Active, omm::ContourStyle(omm::Color(1.0, 1.0, 1.0)));
      set_style(Status::Hovered, omm::ContourStyle(omm::Color(1.0, 0.0, 0.0)));
      set_style(Status::Inactive, omm::ContourStyle(omm::Color(1.0, 0.3, 0.3)));
      break;
    case MoveAxisHandleDirection::Y:
      set_style(Status::Active, omm::ContourStyle(omm::Color(1.0, 1.0, 1.0)));
      set_style(Status::Hovered, omm::ContourStyle(omm::Color(0.0, 1.0, 0.0)));
      set_style(Status::Inactive, omm::ContourStyle(omm::Color(0.3, 1.0, 0.3)));
      break;
    }
  }

  bool contains_global(const arma::vec2& point) const override
  {
    const arma::vec2 global_point = transformation().inverted().apply_to_position(point);
    arma::vec2 o { 0.0, 0.0 };
    arma::vec2 v = project_onto_axis(global_point);

    // clamp v between o and m_direction
    const arma::vec2 min = arma::min(o, m_direction);
    const arma::vec2 max = arma::max(o, m_direction);
    for (uint i : { 0u, 1u }) {
      v(i) = std::clamp(v(i), min(i), max(i));
    }

    return arma::norm(global_point - v) < interact_epsilon();
  }

  bool mouse_move(const arma::vec2& delta, const arma::vec2& pos, const QMouseEvent& e) override
  {
    Handle::mouse_move(delta, pos, e);
    if (status() == Status::Active) {
      auto total_delta = transformation().inverted().apply_to_direction(pos - press_pos());
      total_delta = project_onto_axis(total_delta);
      if (m_tool.integer_transformation()) {
        total_delta = m_tool.viewport_transformation.inverted().apply_to_direction(total_delta);
        static constexpr double step = 10.0;
        const double magnitude = arma::norm(total_delta);
        if (abs(magnitude) > 10e-10) {
          total_delta = (total_delta / magnitude) * static_cast<int>(magnitude / step) * step;
        }
        total_delta = m_tool.viewport_transformation.apply_to_direction(total_delta);
      }
      m_tool.transform_objects_absolute(omm::ObjectTransformation().translated(total_delta), true);
      return true;
    } else {
      return false;
    }
  }

  void draw(omm::AbstractRenderer& renderer) const override
  {
    const double magnitude = arma::norm(m_direction);
    const double argument = std::atan2(m_direction[1], m_direction[0]);

    const omm::Point o(arma::vec2{ 0, 0 });
    const omm::Point tip(m_direction);
    const omm::Point right(PolarCoordinates(argument-0.1, magnitude*0.9).to_cartesian());
    const omm::Point left(PolarCoordinates(argument+0.1, magnitude*0.9).to_cartesian());

    renderer.draw_spline({ o, tip }, current_style());
    renderer.draw_spline({ left, tip, right, left }, current_style());
  }

private:
  ToolT& m_tool;
  arma::vec2 project_onto_axis(const arma::vec2& v) const
  {
    arma::vec2 o { 0.0, 0.0 };
    const arma::vec2 s = m_direction;

    // project v onto the line through o and s
    return o + arma::dot(v-o, s-o) / arma::dot(s-o, s-o) * s;
  }

  const arma::vec2 m_direction;
};

}  // namespace omm
