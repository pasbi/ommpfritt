#include "mainwindow/viewport/axishandle.h"
#include <cmath>
#include "renderers/abstractrenderer.h"
#include "mainwindow/viewport/handle.h"
#include "properties/boolproperty.h"
#include "properties/colorproperty.h"
#include "properties/floatproperty.h"

namespace
{

void draw_arrow( omm::AbstractRenderer& renderer, const omm::Style& style,
                 const arma::vec2& direction )
{
  renderer.draw_spline({ omm::Point({0, 0}), omm::Point(direction)}, style);
}

void set_pen_color(omm::Style& style, const omm::Color& color)
{
  style.property(omm::Style::PEN_IS_ACTIVE_KEY).set(true);
  style.property(omm::Style::BRUSH_IS_ACTIVE_KEY).set(false);
  style.property(omm::Style::PEN_COLOR_KEY).set(color);
  style.property(omm::Style::PEN_WIDTH_KEY).set(2.0);
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
get_transformation( const omm::AxisHandle::Trait::Action action, const arma::vec2& pos,
                    const arma::vec2& delta, const arma::vec2& direction )
{
  omm::ObjectTransformation t;
  switch (action) {
  case omm::AxisHandle::Trait::Action::Translate:
    t.translate(project_onto_axis(delta, direction));
    break;
  case omm::AxisHandle::Trait::Action::Scale:
    t.scale(get_scale(pos, delta, direction));
    break;
  }
  return t;
}

}  // namespace

namespace omm
{

AxisHandle::AxisHandle(Handle& handle, Trait&& trait)
  : SubHandle(handle)
  , m_trait(std::move(trait))
{
}

void AxisHandle::draw(AbstractRenderer& renderer) const
{
  switch (status()) {
  case Status::Active:
    draw_arrow(renderer, m_trait.active_style, m_trait.direction);
    break;
  case Status::Hovered:
    draw_arrow(renderer, m_trait.hover_style, m_trait.direction);
    break;
  case Status::Inactive:
    draw_arrow(renderer, m_trait.base_style, m_trait.direction);
    break;
  }
}

void AxisHandle::mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover)
{
  SubHandle::mouse_move(delta, pos, allow_hover);
  if (status() == Status::Active) {
    handle.transform_objects(get_transformation(m_trait.action, pos, delta, m_trait.direction));
  }
}

bool AxisHandle::contains(const arma::vec2& point) const
{
  constexpr double eps = 10;
  arma::vec2 o { 0.0, 0.0 };
  const arma::vec2 s = m_trait.direction;

  arma::vec2 v = project_onto_axis(point, m_trait.direction);

  // clamp v between o and s
  const arma::vec2 min = arma::min(o, s);
  const arma::vec2 max = arma::max(o, s);
  for (auto i : {0, 1}) {
    v(i) = std::max(static_cast<double>(min(i)), std::min(v(i), static_cast<double>(max(i))));
  }

  return arma::norm(point - v) < eps;
}

XTranslateHandle::XTranslateHandle(Handle& handle): AxisHandle(handle, trait()) {}

AxisHandle::Trait XTranslateHandle::trait()
{
  Trait trait;
  trait.direction = arma::vec2({ 100.0, 0.0 });
  set_pen_color(trait.active_style, Color(1.0, 1.0, 1.0));
  set_pen_color(trait.hover_style, Color(1.0, 0.0, 0.0));
  set_pen_color(trait.base_style, Color(0.5, 0.0, 0.0));
  trait.action = Trait::Action::Translate;
  return trait;
}

YTranslateHandle::YTranslateHandle(Handle& handle): AxisHandle(handle, trait()) {}

AxisHandle::Trait YTranslateHandle::trait()
{
  Trait trait;
  trait.direction = arma::vec2({ 0.0, 100.0 });
  set_pen_color(trait.active_style, Color(1.0, 1.0, 1.0));
  set_pen_color(trait.hover_style, Color(0.0, 1.0, 0.0));
  set_pen_color(trait.base_style, Color(0.0, 0.5, 0.0));
  trait.action = Trait::Action::Translate;
  return trait;
}

XScaleHandle::XScaleHandle(Handle& handle): AxisHandle(handle, trait()) {}

AxisHandle::Trait XScaleHandle::trait()
{
  Trait trait;
  trait.direction = arma::vec2({ -100.0, 0.0 });
  set_pen_color(trait.active_style, Color(1.0, 1.0, 1.0));
  set_pen_color(trait.hover_style, Color(1.0, 1.0, 0.0));
  set_pen_color(trait.base_style, Color(0.5, 0.5, 0.0));
  trait.action = Trait::Action::Scale;
  return trait;
}

YScaleHandle::YScaleHandle(Handle& handle): AxisHandle(handle, trait()) {}

AxisHandle::Trait YScaleHandle::trait()
{
  Trait trait;
  trait.direction = arma::vec2({ 0.0, -100.0 });
  set_pen_color(trait.active_style, Color(1.0, 1.0, 1.0));
  set_pen_color(trait.hover_style, Color(1.0, 1.0, 0.0));
  set_pen_color(trait.base_style, Color(0.5, 0.5, 0.0));
  trait.action = Trait::Action::Scale;
  return trait;
}

ScaleHandle::ScaleHandle(Handle& handle): AxisHandle(handle, trait()) {}

AxisHandle::Trait ScaleHandle::trait()
{
  Trait trait;
  trait.direction = 1.0/sqrt(2.0) * arma::vec2({ 100.0, 100.0 });
  set_pen_color(trait.active_style, Color(1.0, 1.0, 1.0));
  set_pen_color(trait.hover_style, Color(1.0, 1.0, 0.0));
  set_pen_color(trait.base_style, Color(0.5, 0.5, 0.0));
  trait.action = Trait::Action::Scale;
  return trait;
}

}  // namespace omm
