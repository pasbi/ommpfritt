#pragma once

#include <armadillo>
#include "tools/handles/handle.h"

namespace omm
{

class MoveTool;

class MoveAxisHandle : public Handle
{
public:
  MoveAxisHandle(MoveTool& tool);
  void draw(AbstractRenderer& renderer) const override;
  bool contains(const arma::vec2& point) const override;
  void mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover) override;
  void set_direction(const arma::vec2& direction);

private:
  arma::vec2 m_direction;
  MoveTool& m_tool;
};

}  // namespace omm
