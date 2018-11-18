#include "mainwindow/viewport/subhandle.h"
#include "renderers/abstractrenderer.h"

namespace
{

void draw_arrow( omm::AbstractRenderer& renderer, const omm::Style& style,
                 const arma::vec2& tip_position )
{
  constexpr double LENGTH = 100;
  omm::Point center({0, 0});
  omm::Point tip(tip_position);
  renderer.draw_spline({ center, tip }, style);
}

}  // namespace

namespace omm
{

void SubHandle::click(const arma::vec2& pos)
{
  if (contains(pos)) {
    m_is_active = true;
  }
}

void SubHandle::hover(const arma::vec2& pos)
{
  m_is_hovered = contains(pos);
}

void SubHandle::release()
{
  m_is_active = false;
}

bool SubHandle::is_hovered() const
{
  return m_is_hovered;
}

bool SubHandle::is_active() const
{
  return m_is_active;
}

AxisHandle::AxisHandle(Axis axis)
  : m_base_color(axis == Axis::X ? Color::RED : Color::GREEN)
  , m_tip_position(axis == Axis::X ? arma::vec2 { LENGTH, 0 } : arma::vec2 { 0, LENGTH } )
{
}

void AxisHandle::draw(AbstractRenderer& renderer) const
{
  Style style;
  style.is_pen_active = true;
  if (is_hovered() && is_active()) {
    style.pen_color = m_base_color.shaded(1.0);
  } else {
    style.pen_color = m_base_color.shaded(0.8);
  }

  draw_arrow(renderer, style, m_tip_position);
}

bool AxisHandle::contains(const arma::vec2& point) const
{
  return false;
}



}  // namespace omm
