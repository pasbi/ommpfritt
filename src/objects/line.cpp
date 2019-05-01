#include "objects/line.h"
#include <QObject>
#include "properties/floatproperty.h"
#include "properties/boolproperty.h"

namespace omm
{

class Style;

Line::Line(Scene* scene) : AbstractProceduralPath(scene)
{
  static const auto category = QObject::tr("Line").toStdString();
  add_property<FloatProperty>(LENGTH_PROPERTY_KEY, 100.0 )
    .set_label(QObject::tr("length").toStdString()).set_category(category);
  add_property<FloatProperty>(ANGLE_PROPERTY_KEY, 0)
    .set_multiplier(180.0 / M_PI)
    .set_label(QObject::tr("angle").toStdString()).set_category(category);
  add_property<BoolProperty>(CENTER_PROPERTY_KEY, true)
    .set_label(QObject::tr("center").toStdString()).set_category(category);
}

std::string Line::type() const { return TYPE; }
std::unique_ptr<Object> Line::clone() const { return std::make_unique<Line>(*this); }
bool Line::is_closed() const { return false; }

std::vector<Point> Line::points() const
{
  const auto length = property(LENGTH_PROPERTY_KEY)->value<double>();
  const auto angle = property(ANGLE_PROPERTY_KEY)->value<double>();
  const auto centered = property(CENTER_PROPERTY_KEY)->value<bool>();
  const PolarCoordinates a(angle, centered ? -length / 2.0 : 0.0);
  const PolarCoordinates b(angle, centered ?  length / 2.0 : length);
  return std::vector { Point(a.to_cartesian()), Point(b.to_cartesian()) };
}

AbstractPropertyOwner::Flag Line::flags() const
{
  return Object::flags() | Flag::Convertable | Flag::IsPathLike;
}

}  // namespace omm
