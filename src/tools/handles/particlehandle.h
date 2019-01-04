#pragma once

#include <armadillo>
#include "tools/handles/handle.h"

namespace omm
{

class ParticleHandle : public Handle
{
public:
  explicit ParticleHandle();
  bool contains(const arma::vec2& point) const override;
  void draw(omm::AbstractRenderer& renderer) const override;
};

}  // namespace omm
