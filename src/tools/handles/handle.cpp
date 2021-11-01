#include "tools/handles/handle.h"
#include "logging.h"
#include "preferences/uicolors.h"
#include "renderers/painter.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include "tools/tool.h"
#include <QMouseEvent>

namespace omm
{
Handle::Handle(Tool& tool) : tool(tool)
{
}

bool Handle::mouse_press(const Vec2f& pos, const QMouseEvent& event)
{
  m_press_pos = pos;
  if (contains_global(pos)) {
    if (event.button() == Qt::LeftButton) {
      m_status = HandleStatus::Active;
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
  if (m_status != HandleStatus::Active) {
    if (contains_global(pos)) {
      m_status = HandleStatus::Hovered;
    } else {
      m_status = HandleStatus::Inactive;
    }
  }
  if (m_status != old_status) {
    Q_EMIT tool.scene()->mail_box().tool_appearance_changed(tool);
  }
  return false;
}

void Handle::mouse_release(const Vec2f& pos, const QMouseEvent&)
{
  Q_UNUSED(pos);
  m_status = HandleStatus::Inactive;
}

HandleStatus Handle::status() const
{
  return m_status;
}

void Handle::deactivate()
{
  m_status = HandleStatus::Inactive;
}

double Handle::draw_epsilon() const
{
  static constexpr double DRAW_EPSILON = 4.0;
  return DRAW_EPSILON;
}

double Handle::interact_epsilon() const
{
  static constexpr double INTERACT_EPSILON = 4.0;
  return INTERACT_EPSILON;
}

Vec2f Handle::press_pos() const
{
  return m_press_pos;
}

double Handle::discretize(double s, double step)
{
  if (Tool::integer_transformation()) {
    LINFO << s << " " << step;
    return step * static_cast<int>(s / step);
  } else {
    return s;
  }
}

Vec2f Handle::discretize(const Vec2f& vec, bool local, double step) const
{
  if (Tool::integer_transformation()) {
    auto dvec = vec;
    if (!local) {
      dvec = tool.viewport_transformation.inverted().apply_to_direction(vec);
    }
    for (auto i : {0u, 1u}) {
      dvec[i] = Handle::discretize(dvec[i], step);
    }
    if (!local) {
      dvec = tool.viewport_transformation.apply_to_direction(vec);
    }
    return dvec;
  } else {
    return vec;
  }
}

QColor Handle::ui_color(HandleStatus status, const QString& name)
{
  return omm::ui_color(status, "Handle", name);
}

QColor Handle::ui_color(const QString& name) const
{
  return ui_color(status(), name);
}

const std::map<AxisHandleDirection, Vec2f> Handle::axis_directions{
    {AxisHandleDirection::X, Vec2f{100, 0}},
    {AxisHandleDirection::Y, Vec2f{0, 100}},
};

const std::map<AxisHandleDirection, QString> Handle::axis_names{
    {AxisHandleDirection::X, "x-axis"},
    {AxisHandleDirection::Y, "y-axis"},
};

}  // namespace omm
