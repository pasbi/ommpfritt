#include "objects/ellipse.h"

#include "objects/pathobject.h"
#include "path/pathpoint.h"
#include "path/path.h"
#include "path/pathvectorgeometry.h"
#include "properties/boolproperty.h"
#include "properties/floatproperty.h"
#include "properties/floatvectorproperty.h"
#include "properties/integerproperty.h"
#include "scene/scene.h"
#include "scene/disjointpathpointsetforest.h"
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

PathVectorGeometry Ellipse::compute_geometry() const
{
  return {};

//  PathVector path_vector;
//  auto path = std::make_unique<Path>(std::move(points));
//  const auto path_points = path->points();
//  path_vector.add_path(std::move(path));
//  path_vector.joined_points().insert({path_points.front(), path_points.back()});
//  return path_vector;
}

}  // namespace omm
