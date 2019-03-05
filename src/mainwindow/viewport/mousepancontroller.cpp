#include "mainwindow/viewport/mousepancontroller.h"
#include "geometry/objecttransformation.h"
#include <glog/logging.h>

namespace omm
{

MousePanController::MousePanController(const set_cursor_pos_type& set_cursor_pos)
  : m_set_cursor_pos(set_cursor_pos)
{
}

void MousePanController
::start_move(const arma::vec2& viewport_pos, const arma::vec2& global_pos, Action action)
{
  assert(m_action == Action::None);
  m_action = action;
  m_last_position = viewport_pos;
  m_global_start_position = global_pos;
  m_was_applied = false;
}

bool MousePanController::end_move()
{
  m_action = Action::None;
  return m_was_applied;
}

arma::vec2
MousePanController::apply(const arma::vec2& current_cursor_position, ObjectTransformation& t)
{
  m_was_applied = true;
  static constexpr double base_scale = 1.003;
  const arma::vec2 old_cursor_pos = m_last_position;
  const arma::vec2 delta = update(current_cursor_position);

  const int i_max_abs = arma::index_max(arma::abs(delta));
  const double scale = std::pow(base_scale, std::copysign(arma::norm(delta), delta(i_max_abs)));

  if (m_action == Action::Pan) {
    t.translate(delta);
  } else if (m_action == Action::Zoom) {
    t.translate((1-scale) * t.scaling() % m_global_start_position);
    t.scale(arma::vec2{ scale, scale });
  }
  return delta;
}

arma::vec2 MousePanController::update(const arma::vec2& current_cursor_position)
{
  const arma::vec2 delta = current_cursor_position - m_last_position;
  m_last_position = current_cursor_position;
  return delta;
}

}  // namespace omm
