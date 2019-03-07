#include "tools/handles/axishandle.h"
#include <cmath>
#include "renderers/abstractrenderer.h"
#include "tools/handles/handle.h"
#include "properties/boolproperty.h"
#include "properties/colorproperty.h"
#include "properties/floatproperty.h"

namespace omm
{

AxisHandle::AxisHandle(Tool& tool) : Handle(tool, true) {}

bool AxisHandle::contains_global(const arma::vec2& point) const
{
  const arma::vec2 global_point = transformation().inverted().apply_to_position(point);
  arma::vec2 o { 0.0, 0.0 };
  arma::vec2 v = project_onto_axis(global_point);

  // clamp v between o and m_direction
  const arma::vec2 min = arma::min(o, m_direction);
  const arma::vec2 max = arma::max(o, m_direction);
  for (auto i : {0, 1}) {
    v(i) = std::max(static_cast<double>(min(i)), std::min(v(i), static_cast<double>(max(i))));
  }

  return arma::norm(global_point - v) < interact_epsilon();
}

void AxisHandle::set_direction(const arma::vec2& direction) { m_direction = direction; }

arma::vec2 AxisHandle::project_onto_axis(const arma::vec2& v) const
{
  arma::vec2 o { 0.0, 0.0 };
  const arma::vec2 s = m_direction;

  // project v onto the line through o and s
  return o + arma::dot(v-o, s-o) / arma::dot(s-o, s-o) * s;
}

}  // namespace omm
