#include "tools/handles/handle.h"
#include "renderers/abstractrenderer.h"
#include <QMouseEvent>

namespace omm
{

bool Handle::mouse_press(const arma::vec2& pos, const QMouseEvent& event)
{
  if (contains_global(pos)) {
    if (event.button() == Qt::LeftButton) {
      m_status = Status::Active;
    }
    return true;
  } else {
    return false;
  }
}

bool Handle::mouse_move(const arma::vec2& delta, const arma::vec2& pos, const QMouseEvent& event)
{
  if (m_status != Status::Active) {
    if (contains_global(pos)) {
      m_status = Status::Hovered;
    } else {
      m_status = Status::Inactive;
    }
  }
  return false;
}

void Handle::mouse_release(const arma::vec2& pos, const QMouseEvent& event)
{
  m_status = Status::Inactive;
}

Handle::Status Handle::status() const
{
  return m_status;
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

bool Handle::contains_global(const arma::vec2& global_point) const
{
  return contains(transformation().inverted().apply_to_position(global_point));
}

}  // namespace omm
