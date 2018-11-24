#pragma once

#include "mainwindow/viewport/subhandle.h"

namespace omm
{

class CircleHandle : public SubHandle
{
public:
  using SubHandle::SubHandle;
  void draw(AbstractRenderer& renderer) const override;
  bool contains(const arma::vec2& point) const override;
  void mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover) override;

  static constexpr double RADIUS = 100;
};

}  // namespace omm
