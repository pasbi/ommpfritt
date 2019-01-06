#pragma once

#include <armadillo>
#include "tools/handles/handle.h"
#include "renderers/abstractrenderer.h"
#include "geometry/util.h"

namespace omm
{

class AxisHandle : public Handle
{
public:
  bool contains(const arma::vec2& point) const override;
  void set_direction(const arma::vec2& direction);

protected:
  arma::vec2 project_onto_axis(const arma::vec2& v) const;
  arma::vec2 m_direction;
};

template<typename ToolT>
class MoveAxisHandle : public omm::AxisHandle
{
public:
  MoveAxisHandle(ToolT& tool) : m_tool(tool) { }

  void mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover) override
  {
    AxisHandle::mouse_move(delta, pos, allow_hover);
    if (status() == Status::Active) {
      const auto t = omm::ObjectTransformation().translated(project_onto_axis(delta));
      m_tool.transform_objects(t);
    }
  }

  void draw(omm::AbstractRenderer& renderer) const override
  {
    const double magnitude = arma::norm(m_direction);
    const double argument = std::atan2(m_direction[1], m_direction[0]);

    const auto o = omm::Point(arma::vec2{ 0, 0 });
    const auto tip = omm::Point(m_direction);
    const auto right = omm::Point(argument-0.1, magnitude*0.9);
    const auto left = omm::Point(argument+0.1, magnitude*0.9);

    renderer.draw_spline({ o, tip }, current_style());
    renderer.draw_spline({ left, tip, right, left }, current_style());
  }

private:
  ToolT& m_tool;
};

template<typename ItemT>
class ScaleAxisHandle : public omm::AxisHandle
{
public:
  ScaleAxisHandle(ItemT& tool) : m_tool(tool) { }
  void mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover) override
  {
    AxisHandle::mouse_move(delta, pos, allow_hover);
    if (status() == Status::Active) {
      const auto t = omm::ObjectTransformation().scaled(get_scale(pos, delta, m_direction));
      m_tool.transform_objects(t);
    }
  }

  void draw(omm::AbstractRenderer& renderer) const override
  {
    const omm::Point o(arma::vec2{ 0.0, 0.0 });
    const omm::Point tip(m_direction);
    const double r = 3.0;
    const omm::Point tl(m_direction + arma::vec2{-r,  r });
    const omm::Point bl(m_direction + arma::vec2{ r,  r });
    const omm::Point tr(m_direction + arma::vec2{-r, -r });
    const omm::Point br(m_direction + arma::vec2{ r, -r });

    renderer.draw_spline({ o, tip }, current_style());
    renderer.draw_spline({ tl, bl, br, tr, tl }, current_style());
  }

private:
  ItemT& m_tool;
};

}  // namespace omm
