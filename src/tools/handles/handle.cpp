#include "tools/handles/handle.h"
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

Handle::Handle()
{
}

void Handle::mouse_press(const arma::vec2& pos)
{
  if (contains(pos)) {
    m_status = Status::Active;
  }
}

void Handle::mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover)
{
  if (m_status != Status::Active) {
    if (allow_hover && contains(pos)) {
      m_status = Status::Hovered;
    } else {
      m_status = Status::Inactive;
    }
  }
}

void Handle::mouse_release()
{
  m_status = Status::Inactive;
}

Handle::Status Handle::status() const
{
  return m_status;;
}

void Handle::deactivate()
{
  m_status = Status::Inactive;
}

void Handle::set_style(Status status, Style&& style)
{
  m_styles.erase(status);
  m_styles.insert(std::pair(status, style));
}

const Style& Handle::current_style() const
{
  return m_styles.at(m_status);
}


}  // namespace omm
