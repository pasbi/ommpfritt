#include "tools/handles/particlehandle.h"
#include <armadillo>
#include "renderers/abstractrenderer.h"

namespace omm
{

ParticleHandle::ParticleHandle()
{
  set_style(Status::Hovered, omm::SolidStyle(omm::Color(1.0, 1.0, 0.0)));
  set_style(Status::Active, omm::SolidStyle(omm::Color(1.0, 1.0, 1.0)));
  set_style(Status::Inactive, omm::SolidStyle(omm::Color(0.8, 0.8, 0.2)));
}

bool ParticleHandle::contains(const arma::vec2& point) const
{
  return arma::norm(point) < epsilon;
}

void ParticleHandle::draw(omm::AbstractRenderer& renderer) const
{
  renderer.draw_rectangle(arma::vec2{ 0.0, 0.0 }, epsilon, current_style());
}

}  // namespace omm