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

void SubHandle::mouse_press(const arma::vec2& pos)
{
  if (contains(pos)) {
    m_status = Status::Active;
  }
}

void SubHandle::mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover)
{
  if (m_status != Status::Active) {
    if (allow_hover && contains(pos)) {
      m_status = Status::Hovered;
    } else {
      m_status = Status::Inactive;
    }
  }
}

void SubHandle::mouse_release()
{
  m_status = Status::Inactive;
}

SubHandle::Status SubHandle::status() const
{
  return m_status;;
}

void SubHandle::deactivate()
{
  m_status = Status::Inactive;
}

}  // namespace omm
