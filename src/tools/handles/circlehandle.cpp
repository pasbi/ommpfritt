#include "tools/handles/circlehandle.h"
#include "renderers/abstractrenderer.h"
#include "tools/handles/handle.h"
#include "properties/boolproperty.h"
#include "properties/colorproperty.h"

namespace omm
{

CircleHandle::CircleHandle(Tool& tool) : Handle(tool, true) {}

void CircleHandle::draw(AbstractRenderer& renderer) const
{
  constexpr auto n = 200;
  std::vector<omm::Point> points;
  points.reserve(n+1);
  for (size_t i = 0; i <= n; ++i) {
    const double x = cos(i * 2.0/n * M_PI) * m_radius;
    const double y = sin(i * 2.0/n * M_PI) * m_radius;
    points.push_back(omm::Point(arma::vec2 { x, y }));
  }
  renderer.draw_spline(points, current_style());
}

bool CircleHandle::contains_global(const arma::vec2& point) const
{
  // const ObjectTransformation t2 = ObjectTransformation().scaled(arma::vec2{ 1/m_scale(0), 1/m_scale(1) });
  // const ObjectTransformation t1 = transformation().inverted();
  // const auto global_position = t2.apply_to_position(t1.apply_to_position(point));
  const auto global_point = transform_position_to_global(point);
  const arma::vec2 o{ 0.0, 0.0 };
  const double r = arma::norm(global_point - o);
  return RADIUS - interact_epsilon() <= r && r <= RADIUS + interact_epsilon();
}

void CircleHandle::set_radius(double r)
{
  m_radius = r;
}

}  // namespace omm
