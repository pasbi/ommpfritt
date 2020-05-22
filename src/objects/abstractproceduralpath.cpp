#include "objects/abstractproceduralpath.h"
#include <QObject>
#include "properties/floatproperty.h"
#include "objects/path.h"
#include "common.h"
#include "geometry/cubics.h"
#include "renderers/style.h"
#include "objects/tip.h"

namespace omm
{

class Style;

void AbstractProceduralPath::draw_object(Painter &renderer, const Style& style,
                                         Painter::Options options) const
{
  if (QPainter* painter = renderer.painter; painter != nullptr && is_active()) {
    renderer.set_style(style, *this, options);
    painter->drawPath(m_painter_path);
    const auto marker_color = style.property(Style::PEN_COLOR_KEY)->value<Color>();
    const auto width = style.property(Style::PEN_WIDTH_KEY)->value<double>();
    style.start_marker.draw_marker(renderer, evaluate(0.0).rotated(.5 * M_PI), marker_color, width);
    style.end_marker.draw_marker(renderer, evaluate(1.0).rotated(3/2 * M_PI), marker_color, width);
  }
}

Flag AbstractProceduralPath::flags() const
{
  return Object::flags() | Flag::Convertible | Flag::IsPathLike;
}

std::unique_ptr<Object> AbstractProceduralPath::convert() const
{
  auto converted = std::make_unique<Path>(scene());
  copy_properties(*converted, CopiedProperties::Compatible | CopiedProperties::User);
  copy_tags(*converted);
  converted->property(Path::IS_CLOSED_PROPERTY_KEY)->set(is_closed());
  converted->property(Path::INTERPOLATION_PROPERTY_KEY)->set(Path::InterpolationMode::Bezier);
  converted->set_points(m_points);
  return std::unique_ptr<Object>(converted.release());
}

Point AbstractProceduralPath::evaluate(const double t) const
{
  if (m_points.size() > 1) {
    return Cubics(m_points, is_closed()).evaluate(t);
  } else {
    return Point();
  }
}

BoundingBox AbstractProceduralPath::bounding_box(const ObjectTransformation &transformation) const
{
  if (is_active()) {
    return BoundingBox((m_painter_path * transformation.to_qtransform()).boundingRect());
  } else {
    return BoundingBox();
  }
}

double AbstractProceduralPath::path_length() const
{
  return Cubics(m_points, is_closed()).length();
}

bool AbstractProceduralPath::contains(const Vec2f &pos) const
{
  return Cubics(m_points, is_closed()).contains(pos);
}

std::vector<double> AbstractProceduralPath::cut(const Vec2f& c_start, const Vec2f& c_end)
{
  const auto gti = global_transformation(Space::Viewport).inverted();
  return Cubics(m_points, is_closed()).cut( gti.apply_to_position(c_start),
                                            gti.apply_to_position(c_end) );
}

void AbstractProceduralPath::update()
{
  m_points = ::filter_if(points(), [](const auto& point) {
    return !point.has_nan() && !point.has_inf();
  });
  m_painter_path = Painter::path(m_points, is_closed());
  Object::update();
}

}  // namespace omm
