#include "tools/handles/handle.h"
#include "renderers/abstractrenderer.h"
#include <QMouseEvent>
#include "tools/tool.h"

namespace omm
{

Handle::Handle(Tool& tool, const bool transform_in_tool_space)
  : transform_in_tool_space(transform_in_tool_space)
  , tool(tool)
{}

bool Handle::mouse_press(const arma::vec2& pos, const QMouseEvent& event, bool force)
{
  Q_UNUSED(force);
  m_press_pos = pos;
  if (contains_global(pos)) {
    if (event.button() == Qt::LeftButton) {
      m_status = Status::Active;
    }
    return true;
  } else {
    return false;
  }
}

bool Handle::mouse_move(const arma::vec2& delta, const arma::vec2& pos, const QMouseEvent&)
{
  Q_UNUSED(delta);
  if (m_status != Status::Active) {
    if (contains_global(pos)) {
      m_status = Status::Hovered;
    } else {
      m_status = Status::Inactive;
    }
  }
  return false;
}

void Handle::mouse_release(const arma::vec2& pos, const QMouseEvent&)
{
  Q_UNUSED(pos);
  m_status = Status::Inactive;
}

Handle::Status Handle::status() const { return m_status; }
void Handle::deactivate() { m_status = Status::Inactive; }
const Style& Handle::style(Status status) const { return m_styles.at(status); }
const Style& Handle::current_style() const { return style(status()); }
bool Handle::is_enabled() const { return !transform_in_tool_space || tool.has_transformation(); }

void Handle::set_style(Status status, Style style)
{
  m_styles.erase(status);
  m_styles.insert(std::pair(status, std::move(style)));
}

ObjectTransformation Handle::transformation() const
{
  assert(is_enabled());
  if (transform_in_tool_space) {
    return tool.transformation(); // .scaled(m_scale);
  } else {
    return ObjectTransformation(); // .scaled(m_scale);
  }
}

double Handle::draw_epsilon() const { return 4.0; }
double Handle::interact_epsilon() const { return 4.0; }
arma::vec2 Handle::press_pos() const { return m_press_pos; }

arma::vec2 Handle::transform_position_to_global(const arma::vec2& position) const
{
  return transformation().inverted().apply_to_position(position);
}

}  // namespace omm
