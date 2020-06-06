#include "objects/rectangleobject.h"
#include "properties/floatvectorproperty.h"


namespace omm
{

RectangleObject::RectangleObject(Scene *scene)
  : Object(scene)
{
  static const auto category = QObject::tr("rectangle");
  create_property<FloatVectorProperty>(SIZE_PROPERTY_KEY, Vec2f(200.0, 200.0) )
    .set_label(QObject::tr("size")).set_category(category);
  create_property<FloatVectorProperty>(RADIUS_PROPERTY_KEY, Vec2f(1.0, 1.0) )
    .set_step(Vec2f(0.01, 0.01)).set_range(Vec2f(0.0, 0.0), Vec2f(1.0, 1.0))
    .set_label(QObject::tr("radius")).set_category(category);
  create_property<FloatVectorProperty>(TENSION_PROPERTY_KEY, Vec2f(1.0, 1.0) )
    .set_step(Vec2f(0.01, 0.01)).set_range(Vec2f(0.0, 0.0), Vec2f(1.0, 1.0))
    .set_label(QObject::tr("tension")).set_category(category);
  update();
}

QString RectangleObject::type() const { return TYPE; }

Flag RectangleObject::flags() const
{
  return Flag::IsPathLike | Flag::Convertible;
}

Geom::PathVector RectangleObject::paths() const
{
  std::list<Point> points;
  const auto size = property(SIZE_PROPERTY_KEY)->value<Vec2f>()/2.0;
  const auto r = property(RADIUS_PROPERTY_KEY)->value<Vec2f>();
  const auto t = property(TENSION_PROPERTY_KEY)->value<Vec2f>();
  const Vec2f ar(size.x * r.x, size.y * r.y);

  const PolarCoordinates null(0.0, 0.0);
  const PolarCoordinates v(Vec2f(0.0, -ar.y*t.y));
  const PolarCoordinates h(Vec2f(ar.x*t.x, 0.0));

  const bool p = ar != Vec2f::o();
  if (p) {
    points.push_back(Point(Vec2f(-size.x+ar.x, -size.y     ), null, -h));
  }
  points.push_back(Point(Vec2f(-size.x,      -size.y+ar.y), v, null));
  if (p) {
    points.push_back(Point(Vec2f(-size.x,       size.y-ar.y), null, -v));
  }
  points.push_back(Point(Vec2f(-size.x+ar.x,  size.y     ), -h, null));
  if (p) {
    points.push_back(Point(Vec2f( size.x-ar.x,  size.y     ), null, h));
  }
  points.push_back(Point(Vec2f( size.x,       size.y-ar.y), -v, null));
  if (p) {
    points.push_back(Point(Vec2f( size.x,      -size.y+ar.y), null, v));
  }
  points.push_back(Point(Vec2f( size.x-ar.x, -size.y     ), h, null));

  const Segment segment(points.begin(), points.end());
  const auto path = segment_to_path(segment, is_closed());
  return Geom::PathVector(path);
}

void RectangleObject::on_property_value_changed(Property *property)
{
  if (   property == this->property(SIZE_PROPERTY_KEY)
      || property == this->property(RADIUS_PROPERTY_KEY)
      || property == this->property(TENSION_PROPERTY_KEY))
  {
    update();
  } else {
    Object::on_property_value_changed(property);
  }
}

}  // namespace omm
