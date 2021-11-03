#include "objects/ellipse.h"

#include "objects/path.h"
#include "objects/segment.h"
#include "properties/boolproperty.h"
#include "properties/floatproperty.h"
#include "properties/floatvectorproperty.h"
#include "properties/integerproperty.h"
#include "scene/scene.h"
#include <QObject>

namespace omm
{
class Style;

Ellipse::Ellipse(Scene* scene) : Object(scene)
{
  static constexpr double DEFAULT_RADIUS = 100.0;
  static constexpr int DEFAULT_CORNER_COUNT = 12;
  static const auto category = QObject::tr("ellipse");
  create_property<FloatVectorProperty>(RADIUS_PROPERTY_KEY, Vec2f(DEFAULT_RADIUS, DEFAULT_RADIUS))
      .set_label(QObject::tr("r"))
      .set_category(category);
  create_property<IntegerProperty>(CORNER_COUNT_PROPERTY_KEY, DEFAULT_CORNER_COUNT)
      .set_range(3, IntegerProperty::highest_possible_value())
      .set_label(QObject::tr("n"))
      .set_category(category);
  create_property<BoolProperty>(SMOOTH_PROPERTY_KEY, true)
      .set_label(QObject::tr("smooth"))
      .set_category(category);
  update();
}

QString Ellipse::type() const
{
  return TYPE;
}

void Ellipse::on_property_value_changed(Property* property)
{
  if (property == this->property(RADIUS_PROPERTY_KEY)
      || property == this->property(CORNER_COUNT_PROPERTY_KEY)
      || property == this->property(SMOOTH_PROPERTY_KEY)) {
    update();
  } else {
    Object::on_property_value_changed(property);
  }
}

Geom::PathVector Ellipse::paths() const
{
  const auto n_raw = property(CORNER_COUNT_PROPERTY_KEY)->value<int>();
  const auto n = static_cast<std::size_t>(std::max(3, n_raw));
  const auto r = property(RADIUS_PROPERTY_KEY)->value<Vec2f>();
  const bool smooth = property(SMOOTH_PROPERTY_KEY)->value<bool>();
  std::deque<std::unique_ptr<Point>> points;
  for (std::size_t i = 0; i < n; ++i) {
    const double theta = i * 2.0 / n * M_PI;
    const double x = std::cos(theta) * r.x;
    const double y = std::sin(theta) * r.y;
    if (smooth) {
      const Vec2f d(std::sin(theta) * r.x, -std::cos(theta) * r.y);
      points.emplace_back(std::make_unique<Point>(Vec2f{x, y}, d.arg(), 2.0 * d.euclidean_norm() / n));
    } else {
      points.emplace_back(std::make_unique<Point>(Vec2f{x, y}));
    }
  }
  return Geom::PathVector{Segment(std::move(points)).to_geom_path(true)};
}

}  // namespace omm
