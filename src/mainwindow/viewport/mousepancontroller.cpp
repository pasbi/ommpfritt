#include "mainwindow/viewport/mousepancontroller.h"
#include "geometry/objecttransformation.h"

namespace omm
{
MousePanController::MousePanController(const set_cursor_pos_type& set_cursor_pos)
    : m_set_cursor_pos(set_cursor_pos)
{
}

void MousePanController::start_move(const Vec2f& global_pos, Action action)
{
  if (m_action == Action::None) {
    m_action = action;
    m_global_start_position = global_pos;
    m_was_applied = false;
  }
}

bool MousePanController::end_move()
{
  m_action = Action::None;
  return m_was_applied;
}

Vec2f MousePanController::apply(const Vec2f& delta, ObjectTransformation& t)
{
  m_was_applied = true;
  static constexpr double base_scale = 1.003;

  const double max_mag = std::abs(delta.x) > std::abs(delta.y) ? delta.x : delta.y;
  const double scale = std::pow(base_scale, std::copysign(delta.euclidean_norm(), max_mag));

  if (m_action == Action::Pan) {
    t.translate(delta);
  } else if (m_action == Action::Zoom) {
    t = t.apply(ObjectTransformation().translated((1 - scale) * m_global_start_position));
    t.scale(Vec2f(scale, scale));
  }
  return delta;
}

}  // namespace omm
