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
  const auto d = arma::norm(point - transformation().apply_to_position(position));
  LOG(INFO) << d;
  return d < interact_epsilon();
  // return arma::norm(transform_position_to_global(point) - position) < interact_epsilon();
}

void ParticleHandle::draw(omm::AbstractRenderer& renderer) const
{
  // LOG(INFO) << renderer.current_transformation();
  // LOG(INFO) << draw_epsilon();
  renderer.push_transformation(ObjectTransformation().translated(position));
  // renderer.push_transformation(ObjectTransformation().scaled(m_scale));
  renderer.draw_rectangle(arma::vec2{ 0.0, 0.0 }, draw_epsilon(), current_style());
  // renderer.pop_transformation();
  renderer.pop_transformation();
}

}  // namespace omm
