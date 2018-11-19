#include "mainwindow/viewport/subhandle.h"
#include "renderers/abstractrenderer.h"
#include "mainwindow/viewport/handle.h"

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

SubHandle::SubHandle(Handle& handle)
  : handle(handle)
{
}

bool SubHandle::mouse_press(const arma::vec2& pos)
{
  if (contains(pos)) {
    m_is_active = true;
    return true;
  } else {
    return false;
  }
}

bool SubHandle::mouse_move(const arma::vec2& delta, const arma::vec2& pos)
{
  m_is_hovered = !m_is_active && contains(pos);
  return m_is_active;
}

void SubHandle::mouse_release()
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

AxisHandle::AxisHandle(Handle& handle, Axis axis)
  : SubHandle(handle)
  , m_base_color(axis == Axis::X ? Color::RED : Color::GREEN)
  , m_tip_position(axis == Axis::X ? arma::vec2 { LENGTH, 0 } : arma::vec2 { 0, LENGTH } )
{
}

void AxisHandle::draw(AbstractRenderer& renderer) const
{
  Style style;
  style.is_pen_active = true;
  if (is_active()) {
    style.pen_color = Color::WHITE;
  } else if (is_hovered()) {
    style.pen_color = m_base_color.shaded(1.0);
  } else {
    style.pen_color = m_base_color.shaded(0.8);
  }

  draw_arrow(renderer, style, m_tip_position);
}

arma::vec2 AxisHandle::project(const arma::vec2& point) const
{
  arma::vec2 o { 0.0, 0.0 };
  arma::vec2 v = point;
  const arma::vec2 s = m_tip_position;

  // project v onto the line through o and s
  return o + arma::dot(v-o, s-o) / arma::dot(s-o, s-o) * s;
}

bool AxisHandle::mouse_move(const arma::vec2& delta, const arma::vec2& pos)
{
  if (SubHandle::mouse_move(delta, pos)) {
    handle.transform_objects(ObjectTransformation().translated(project(delta)));
    return true;
  } else {
    return false;
  }
}

bool AxisHandle::contains(const arma::vec2& point) const
{
  constexpr double eps = 10;
  arma::vec2 o { 0.0, 0.0 };
  const arma::vec2 s = m_tip_position;

  arma::vec2 v = project(point);

  // clamp v between o and s
  const arma::vec2 min = arma::min(o, s);
  const arma::vec2 max = arma::max(o, s);
  for (auto i : {0, 1}) {
    v(i) = std::max(static_cast<double>(min(i)), std::min(v(i), static_cast<double>(max(i))));
  }

  return arma::norm(point - v) < eps;
}



}  // namespace omm
