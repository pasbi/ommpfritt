#pragma once

#include <functional>
#include <armadillo>

namespace omm
{

class MousePanController
{
private:
  using set_cursor_pos_type = std::function<void(const arma::vec2& pos)>;
public:
  explicit MousePanController(const set_cursor_pos_type& set_cursor_pos);
  void init(const arma::vec2& pos);
  arma::vec2 delta(arma::vec2 pos, const arma::vec2& max);

private:
  arma::vec2 m_last_position;
  const set_cursor_pos_type m_set_cursor_pos;
};

}  // namespace omm
