#include "tools/handles/axishandle.h"
#include <cmath>
#include "renderers/abstractrenderer.h"
#include "tools/handles/handle.h"
#include "properties/boolproperty.h"
#include "properties/colorproperty.h"
#include "properties/floatproperty.h"
#include "tools/movetool.h"

namespace
{

void draw_arrow( omm::AbstractRenderer& renderer, const omm::Style& style,
                 const arma::vec2& direction )
{
  renderer.draw_spline({ omm::Point({0, 0}), omm::Point(direction)}, style);
}

arma::vec2 project_onto_axis(const arma::vec2 v, const arma::vec2& axis_direction)
{
  arma::vec2 o { 0.0, 0.0 };
  const arma::vec2 s = axis_direction;

  // project v onto the line through o and s
  return o + arma::dot(v-o, s-o) / arma::dot(s-o, s-o) * s;
}

arma::vec2 get_scale(const arma::vec2& pos, const arma::vec2& delta, const arma::vec2& direction)
{
  const auto factor = [](const double new_pos, const double delta, const double constraint) {
    const double old_pos = new_pos - delta;
    const double epsilon = 0.0001;
    if (std::abs(old_pos) < epsilon) {
      return 1.0;
    } else {
      double s = new_pos / old_pos;
      s = std::copysign(std::pow(std::abs(s), std::abs(constraint)), s);
      return s;
      if (std::abs(s) < 0.1) {
        return std::copysign(1.0, s);
      } else {
        return s;
      }
    }
  };
  const arma::vec old_pos = pos - delta;
  const arma::vec d = direction / arma::norm(direction);
  return arma::vec({ factor(pos(0), delta(0), d(0)), factor(pos(1), delta(1), d(1)) });
}

omm::ObjectTransformation
get_transformation(const arma::vec2& pos, const arma::vec2& delta, const arma::vec2& direction)
{
  return omm::ObjectTransformation().translated(project_onto_axis(delta, direction));
  // t.scale(get_scale(pos, delta, direction));
}

}  // namespace

namespace omm
{

MoveAxisHandle::MoveAxisHandle(MoveTool& tool)
  : m_tool(tool)
{
}

void MoveAxisHandle::draw(AbstractRenderer& renderer) const
{
  draw_arrow(renderer, current_style(), m_direction);
}

void
MoveAxisHandle::mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover)
{
  Handle::mouse_move(delta, pos, allow_hover);
  if (status() == Status::Active) {
    m_tool.transform_objects(get_transformation(pos, delta, m_direction));
  }
}

bool MoveAxisHandle::contains(const arma::vec2& point) const
{
  constexpr double eps = 10;
  arma::vec2 o { 0.0, 0.0 };

  arma::vec2 v = project_onto_axis(point, m_direction);

  // clamp v between o and m_direction
  const arma::vec2 min = arma::min(o, m_direction);
  const arma::vec2 max = arma::max(o, m_direction);
  for (auto i : {0, 1}) {
    v(i) = std::max(static_cast<double>(min(i)), std::min(v(i), static_cast<double>(max(i))));
  }

  return arma::norm(point - v) < eps;
}

void MoveAxisHandle::set_direction(const arma::vec2& direction)
{
  m_direction = direction;
}

}  // namespace omm
