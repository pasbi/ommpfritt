#pragma once

#include "tools/handles/handle.h"

namespace omm
{

class CircleHandle : public Handle
{
public:
  using Handle::Handle;
  void draw(AbstractRenderer& renderer) const override;
  bool contains(const arma::vec2& point) const override;
  void set_radius(double r);

  static constexpr double RADIUS = 100;
private:
  double m_radius;
};

}  // namespace omm
