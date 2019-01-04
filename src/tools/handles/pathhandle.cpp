#include "tools/handles/pathhandle.h"
#include <armadillo>
#include "renderers/abstractrenderer.h"
#include "objects/path.h"

namespace omm
{


PathHandle::PathHandle(Path& path)
  : m_path(path)
{
  set_style(Status::Hovered, omm::SolidStyle(omm::Color(1.0, 1.0, 0.0)));
  set_style(Status::Active, omm::SolidStyle(omm::Color(1.0, 1.0, 1.0)));
  set_style(Status::Inactive, omm::SolidStyle(omm::Color(0.8, 0.8, 0.2)));
}

void PathHandle::draw(omm::AbstractRenderer& renderer) const
{
  renderer.push_transformation(m_path.global_transformation());
  for (auto&& point : m_path.points()) {
    renderer.draw_rectangle(point->position, epsilon, current_style());
  }
  renderer.pop_transformation();
}

bool PathHandle::contains(const arma::vec2& candidate) const
{
  const auto t_inv = m_path.global_transformation().inverted();
  const auto local_candidate = t_inv.apply_to_position(candidate);
  const auto points = m_path.points();
  const auto predicate = [eps=epsilon, &local_candidate](const Point* point) {
    return arma::norm(local_candidate - point->position) < eps;
  };
  return std::any_of(points.begin(), points.end(), predicate);
}

}  // namespace omm
