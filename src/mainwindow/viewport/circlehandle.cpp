#include "mainwindow/viewport/circlehandle.h"
#include "renderers/abstractrenderer.h"
#include "mainwindow/viewport/handle.h"
#include "properties/boolproperty.h"
#include "properties/colorproperty.h"

namespace
{

void draw_circle(omm::AbstractRenderer& renderer, const omm::Style& style)
{
  constexpr auto n = 200;
  std::vector<omm::Point> points;
  points.reserve(n+1);
  for (size_t i = 0; i <= n; ++i) {
    const double x = cos(i * 2.0/n * M_PI) * omm::CircleHandle::RADIUS;
    const double y = sin(i * 2.0/n * M_PI) * omm::CircleHandle::RADIUS;
    points.push_back(omm::Point(arma::vec2 { x, y }));
  }
  renderer.draw_spline(points, style);
}

}  // namespace

namespace omm
{

void CircleHandle::draw(AbstractRenderer& renderer) const
{
  Style style;
  style.property(Style::PEN_IS_ACTIVE_KEY).set(true);
  switch (status()) {
  case Status::Active:
    style.property(Style::PEN_COLOR_KEY).set(Color::WHITE);
    break;
  case Status::Hovered:
    style.property(Style::PEN_COLOR_KEY).set(Color(0.2, 0.3, 1.0));
    break;
  case Status::Inactive:
    style.property(Style::PEN_COLOR_KEY).set(Color(0.0, 0.0, 1.0));
    break;
  }
  draw_circle(renderer, style);
}

void
CircleHandle::mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover)
{
  SubHandle::mouse_move(delta, pos, allow_hover);
  if (status() == Status::Active) {
    const arma::vec2 origin = pos - delta;
    double origin_angle = atan2(origin(1), origin(0));
    double pos_angle = atan2(pos(1), pos(0));
    handle.transform_objects(ObjectTransformation().rotated(pos_angle - origin_angle));
  }
}

bool CircleHandle::contains(const arma::vec2& point) const
{
  constexpr double eps = 10;
  const double r = arma::norm(point);
  return RADIUS - eps <= r && r <= RADIUS + eps;
}

}  // namespace omm
