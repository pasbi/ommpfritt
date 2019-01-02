#include "tools/handles/pointhandle.h"
#include <armadillo>
#include "renderers/abstractrenderer.h"

namespace omm
{

PointHandle::PointHandle()
{
  set_style(Status::Hovered, omm::Style(omm::Color(1.0, 1.0, 0.0)));
  set_style(Status::Active, omm::Style(omm::Color(1.0, 1.0, 1.0)));
  set_style(Status::Inactive, omm::Style(omm::Color(0.8, 0.8, 0.2)));
}

bool PointHandle::contains(const arma::vec2& point) const
{
  return arma::norm(point) < epsilon;
}

void PointHandle::draw(omm::AbstractRenderer& renderer) const
{
  const auto tl = Point( arma::vec2 { -epsilon, -epsilon } );
  const auto tr = Point( arma::vec2 {  epsilon, -epsilon } );
  const auto bl = Point( arma::vec2 { -epsilon,  epsilon } );
  const auto br = Point( arma::vec2 {  epsilon,  epsilon } );
  renderer.draw_spline({ tl, tr, br, bl }, current_style(), true);
}

}  // namespace omm