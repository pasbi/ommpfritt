#pragma once

#include <functional>
#include <armadillo>

namespace omm
{

class ObjectTransformation;

class MousePanController
{
private:
  using set_cursor_pos_type = std::function<void(const arma::vec2& pos)>;
public:
  explicit MousePanController(const set_cursor_pos_type& set_cursor_pos);
  enum class Action { None, Pan, Zoom };
  void start_move(const arma::vec2& viewport_pos, const arma::vec2& global_pos, Action actiono);
  bool end_move();
  arma::vec2 apply(const arma::vec2& current_cursor_position, ObjectTransformation& t);
  arma::vec2 update(const arma::vec2& current_cursor_position);

private:
  arma::vec2 m_last_position;
  arma::vec2 m_global_start_position;
  Action m_action = Action::None;
  const set_cursor_pos_type m_set_cursor_pos;
  bool m_was_applied = false;
};

}  // namespace omm
