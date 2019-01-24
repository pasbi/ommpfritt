#include "tools/handles/particlehandle.h"
#include <armadillo>
#include "renderers/abstractrenderer.h"

namespace omm
{

ParticleHandle::ParticleHandle(Tool& tool, bool transform_in_tool_space)
  : Handle(tool, transform_in_tool_space)
{
  set_style(Status::Hovered, omm::SolidStyle(omm::Color(1.0, 1.0, 0.0)));
  set_style(Status::Active, omm::SolidStyle(omm::Color(1.0, 1.0, 1.0)));
  set_style(Status::Inactive, omm::SolidStyle(omm::Color(0.8, 0.8, 0.2)));
}

bool ParticleHandle::contains_global(const arma::vec2& point) const
{
  return arma::norm(point - transformation().apply_to_position(position)) < epsilon;
}

void ParticleHandle::draw(omm::AbstractRenderer& renderer) const
{
  renderer.draw_rectangle(position, epsilon, current_style());
}

}  // namespace omm
