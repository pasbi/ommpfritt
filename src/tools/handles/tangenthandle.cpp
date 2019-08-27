#include "tools/handles/tangenthandle.h"
#include "tools/handles/selecthandle.h"

namespace omm
{

TangentHandle::TangentHandle(Tool& tool, PointSelectHandle& master_handle, Tangent tangent)
  : Handle(tool), m_master_handle(master_handle), m_tangent(tangent)
{
  set_style(Handle::Status::Hovered, []() {
    Style style;
    style.property(Style::COSMETIC_KEY)->set(true);
    style.property(Style::BRUSH_COLOR_KEY)->set(Color(1.0, 1.0, 0.0));
    style.property(Style::PEN_COLOR_KEY)->set(Color(0.0, 0.0, 1.0));
    style.property(Style::BRUSH_IS_ACTIVE_KEY)->set(true);
    style.property(Style::PEN_IS_ACTIVE_KEY)->set(true);
    return style;
  }());

  set_style(Handle::Status::Active, []() {
    Style style;
    style.property(Style::COSMETIC_KEY)->set(true);
    style.property(Style::BRUSH_COLOR_KEY)->set(Color(1.0, 1.0, 1.0));
    style.property(Style::PEN_COLOR_KEY)->set(Color(0.0, 0.0, 0.0));
    style.property(Style::BRUSH_IS_ACTIVE_KEY)->set(true);
    style.property(Style::PEN_IS_ACTIVE_KEY)->set(true);
    return style;
  }());

  set_style(Handle::Status::Inactive, []() {
    Style style;
    style.property(Style::COSMETIC_KEY)->set(true);
    style.property(Style::BRUSH_COLOR_KEY)->set(Color(0.8, 0.8, 0.2));
    style.property(Style::PEN_COLOR_KEY)->set(Color(0.2, 0.2, 0.8));
    style.property(Style::BRUSH_IS_ACTIVE_KEY)->set(true);
    style.property(Style::PEN_IS_ACTIVE_KEY)->set(true);
    return style;
  }());
}

double TangentHandle::draw_epsilon() const { return 2.0; }

void TangentHandle::draw(Painter& renderer) const
{
  renderer.set_style(current_style());
  const auto r = draw_epsilon();
  renderer.painter->drawEllipse(position.x - r, position.y - r, 2*r, 2*r);
}

bool TangentHandle::contains_global(const Vec2f &point) const
{
  const double dist = (point - position).euclidean_norm();
  return dist < interact_epsilon();
}

bool TangentHandle::mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e)
{
  Handle::mouse_move(delta, pos, e);
  if (status() == Status::Active) {
    m_master_handle.transform_tangent(delta, m_tangent);
    return true;
  } else {
    return false;
  }
}

}  // namespace omm
