#include "mainwindow/viewport/mousepancontroller.h"

namespace omm
{

MousePanController::MousePanController(const set_cursor_pos_type& set_cursor_pos)
  : m_set_cursor_pos(set_cursor_pos)
{
}

void MousePanController::init(const arma::vec2& pos)
{
  m_last_position = pos;
}

arma::vec2 MousePanController::delta(arma::vec2 new_position, const arma::vec2& max)
{
  const decltype(m_last_position) delta = new_position - m_last_position;

  const auto infinity_pan_shift = [](double& coordinate, const double max) {
    while (coordinate >= max) {
      coordinate -= max;
    }
    while (coordinate <= 0) {
      coordinate += max;
    }
  };

  infinity_pan_shift(new_position(0), max(0));
  infinity_pan_shift(new_position(1), max(1));

  m_set_cursor_pos(new_position);
  m_last_position = new_position;

  return delta;
}

}  // namespace omm
