#include "tools/handles/handle.h"
#include "renderers/painter.h"
#include <QMouseEvent>
#include "tools/tool.h"
#include "scene/scene.h"

namespace omm
{

Handle::Handle(Tool& tool, const bool transform_in_tool_space)
  : transform_in_tool_space(transform_in_tool_space)
  , tool(tool)
{}

bool Handle::mouse_press(const Vec2f& pos, const QMouseEvent& event, bool force)
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

bool Handle::mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent&)
{
  const auto old_status = m_status;
  Q_UNUSED(delta);
  if (m_status != Status::Active) {
    if (contains_global(pos)) {
      m_status = Status::Hovered;
    } else {
      m_status = Status::Inactive;
    }
  }
  if (m_status != old_status) {
    tool.scene.repaint();
  }
  return false;
}

void Handle::mouse_release(const Vec2f& pos, const QMouseEvent&)
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
Vec2f Handle::press_pos() const { return m_press_pos; }

Vec2f Handle::transform_position_to_global(const Vec2f& position) const
{
  return transformation().inverted().apply_to_position(position);
}

void Handle::discretize(Vec2f& vec) const
{
  if (tool.integer_transformation()) {
    vec = tool.viewport_transformation.inverted().apply_to_direction(vec);
    static constexpr double step = 10.0;
    for (auto i : { 0u, 1u }) {
      vec[i] = step * static_cast<int>(vec[i] / step);
    }
    vec = tool.viewport_transformation.apply_to_direction(vec);
  }
}

}  // namespace omm
