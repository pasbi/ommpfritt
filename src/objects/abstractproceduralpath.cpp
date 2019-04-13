#include "objects/abstractproceduralpath.h"
#include <QObject>
#include "properties/floatproperty.h"
#include "objects/path.h"
#include "common.h"
#include "geometry/cubics.h"

namespace omm
{

class Style;

void AbstractProceduralPath::draw_object(AbstractRenderer& renderer, const Style& style)
{
  if (is_active()) { renderer.draw_spline(points(), style, is_closed()); }
}

BoundingBox AbstractProceduralPath::bounding_box()
{
  return BoundingBox(points());
}

AbstractPropertyOwner::Flag AbstractProceduralPath::flags() const
{
  return Object::flags() | Flag::Convertable | Flag::IsPathLike;
}

std::unique_ptr<Object> AbstractProceduralPath::convert()
{
  auto converted = std::make_unique<Path>(scene());
  copy_properties(*converted);
  copy_tags(*converted);
  converted->property(Path::IS_CLOSED_PROPERTY_KEY).set(is_closed());
  converted->property(Path::INTERPOLATION_PROPERTY_KEY).set(Path::InterpolationMode::Bezier);
  converted->set_points(points());
  return std::unique_ptr<Object>(converted.release());
}

Point AbstractProceduralPath::evaluate(const double t)
{
  return Cubics(points(), is_closed()).evaluate(t);
}

double AbstractProceduralPath::path_length() { return Cubics(points(), is_closed()).length(); }

bool AbstractProceduralPath::contains(const Vec2f &pos)
{
  return Cubics(points(), is_closed()).contains(pos);
}

std::vector<double> AbstractProceduralPath::cut(const Vec2f& c_start, const Vec2f& c_end)
{
  const auto gti = global_transformation().inverted();
  return Cubics(points(), is_closed()).cut( gti.apply_to_position(c_start),
                                            gti.apply_to_position(c_end) );
}

}  // namespace omm
