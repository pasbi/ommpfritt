#include "tools/handles/handle.h"
#include "renderers/abstractrenderer.h"

namespace omm
{

Handle::Handle()
{
}

bool Handle::mouse_press(const arma::vec2& pos)
{
  if (contains(transformation().inverted().apply_to_position(pos))) {
    m_status = Status::Active;
    return true;
  } else {
    return false;
  }
}

void Handle::mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover)
{
  if (m_status != Status::Active) {
    if (allow_hover && contains(transformation().inverted().apply_to_position(pos))) {
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
  return style(status());
}

const Style& Handle::style(Status status) const
{
  return m_styles.at(status);
}

ObjectTransformation Handle::transformation() const
{
  return ObjectTransformation();
}

}  // namespace omm
