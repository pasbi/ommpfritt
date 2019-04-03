#include "objects/ellipse.h"
#include <QObject>
#include "properties/floatproperty.h"
#include "objects/path.h"
#include "properties/vectorproperty.h"
#include "properties/integerproperty.h"
#include "properties/boolproperty.h"

namespace omm
{

class Style;

Ellipse::Ellipse(Scene* scene) : AbstractProceduralPath(scene)
{
  static const auto category = QObject::tr("ellipse").toStdString();
  add_property<FloatVectorProperty>(RADIUS_PROPERTY_KEY, Vec2f(100.0, 100.0) )
    .set_label(QObject::tr("r").toStdString()).set_category(category);
  add_property<IntegerProperty>(CORNER_COUNT_PROPERTY_KEY, 100)
    .set_range(3, IntegerPropertyLimits::upper)
    .set_label(QObject::tr("n").toStdString()).set_category(category);
  add_property<BoolProperty>(SMOOTH_PROPERTY_KEY, true)
    .set_label(QObject::tr("smooth").toStdString()).set_category(category);
}

BoundingBox Ellipse::bounding_box()
{
  const auto r = property(RADIUS_PROPERTY_KEY).value<Vec2f>();
  return BoundingBox({ -r, r });
}

std::string Ellipse::type() const
{
  return TYPE;
}

std::unique_ptr<Object> Ellipse::clone() const
{
  return std::make_unique<Ellipse>(*this);
}

std::vector<Point> Ellipse::points()
{
  const auto n_raw = property(CORNER_COUNT_PROPERTY_KEY).value<int>();
  const auto n = static_cast<std::size_t>(std::max(3, n_raw));
  const auto r = property(RADIUS_PROPERTY_KEY).value<Vec2f>();
  const bool smooth = property(SMOOTH_PROPERTY_KEY).value<bool>();
  std::vector<Point> points;
  points.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    const double theta = i * 2.0/n * M_PI;
    const double x = std::cos(theta) * r.x;
    const double y = std::sin(theta) * r.y;
    if (smooth) {
      const Vec2f d(std::sin(theta) * r.x, -std::cos(theta) * r.y);
      points.push_back(Point(Vec2f(x, y), std::atan2(d.y, d.x), 2.0 * d.euclidean_norm()/n));
    } else {
      points.push_back(Point(Vec2f(x, y)));
    }
  }
  return points;
}

bool Ellipse::is_closed() const
{
  return true;
}

AbstractPropertyOwner::Flag Ellipse::flags() const
{
  return Object::flags() | Flag::Convertable | Flag::IsPathLike;
}


}  // namespace omm
