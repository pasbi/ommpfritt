#pragma once

#include <armadillo>
#include "tools/handles/handle.h"

namespace omm
{

class PointHandle : public Handle
{
public:
  explicit PointHandle();
  bool contains(const arma::vec2& point) const override;
  void draw(omm::AbstractRenderer& renderer) const override;
};

}  // namespace omm
