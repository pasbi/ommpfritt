#include "mainwindow/viewport/mousepancontroller.h"
#include "geometry/objecttransformation.h"

namespace omm
{

MousePanController::MousePanController(const set_cursor_pos_type& set_cursor_pos)
  : m_set_cursor_pos(set_cursor_pos)
{
}

void MousePanController::init(const arma::vec2& pos, Action action)
{
  m_action = action;
  m_last_position = pos;
}

arma::vec2
MousePanController::apply(const arma::vec2& current_cursor_position, ObjectTransformation& t)
{
  const arma::vec2 old_cursor_pos = m_last_position;
  const arma::vec2 delta = update(current_cursor_position);
  t.translate(delta);
  return delta;
}

arma::vec2 MousePanController::update(const arma::vec2& current_cursor_position)
{
  const arma::vec2 delta = current_cursor_position - m_last_position;
  m_last_position = current_cursor_position;
  return delta;
}

}  // namespace omm
