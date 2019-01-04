#include "tools/handles/pointshandle.h"
#include <armadillo>
#include "renderers/abstractrenderer.h"

namespace omm
{


PointsHandle::PointsHandle(std::vector<Point*>& points)
  : m_points(points)
{
  set_style(Status::Hovered, omm::SolidStyle(omm::Color(1.0, 1.0, 0.0)));
  set_style(Status::Active, omm::SolidStyle(omm::Color(1.0, 1.0, 1.0)));
  set_style(Status::Inactive, omm::SolidStyle(omm::Color(0.8, 0.8, 0.2)));
}

void PointsHandle::draw(omm::AbstractRenderer& renderer) const
{
  // renderer.draw_rectangle(m_point.position, epsilon, current_style());
}

bool PointsHandle::contains(const arma::vec2& point) const
{
  return false;
  // return arma::norm(point) < epsilon;
}

}  // namespace omm