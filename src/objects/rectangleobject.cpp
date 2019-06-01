#include "objects/rectangleobject.h"
#include "properties/vectorproperty.h"


namespace omm
{

RectangleObject::RectangleObject(Scene *scene)
  : AbstractProceduralPath(scene)
{
  static const auto category = QObject::tr("rectangle").toStdString();
  add_property<FloatVectorProperty>(SIZE_PROPERTY_KEY, Vec2f(200.0, 200.0) )
    .set_label(QObject::tr("size").toStdString()).set_category(category);
  add_property<FloatVectorProperty>(RADIUS_PROPERTY_KEY, Vec2f(1.0, 1.0) )
    .set_step(Vec2f(0.01, 0.01)).set_range(Vec2f(0.0, 0.0), Vec2f(1.0, 1.0))
    .set_label(QObject::tr("radius").toStdString()).set_category(category);
  add_property<FloatVectorProperty>(TENSION_PROPERTY_KEY, Vec2f(1.0, 1.0) )
    .set_step(Vec2f(0.01, 0.01)).set_range(Vec2f(0.0, 0.0), Vec2f(1.0, 1.0))
    .set_label(QObject::tr("tension").toStdString()).set_category(category);
}

std::string RectangleObject::type() const { return TYPE; }

std::unique_ptr<Object> RectangleObject::clone() const
{
  return std::make_unique<RectangleObject>(*this);
}

AbstractPropertyOwner::Flag RectangleObject::flags() const
{
  return Object::Flag::IsPathLike | Object::Flag::Convertable;
}

std::vector<Point> RectangleObject::points() const
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
  if (p) { points.push_back(Point(Vec2f(-size.x+ar.x, -size.y     ), null, -h)); }
  points.push_back(Point(Vec2f(-size.x,      -size.y+ar.y), v, null));
  if (p) { points.push_back(Point(Vec2f(-size.x,       size.y-ar.y), null, -v)); }
  points.push_back(Point(Vec2f(-size.x+ar.x,  size.y     ), -h, null));
  if (p) { points.push_back(Point(Vec2f( size.x-ar.x,  size.y     ), null, h)); }
  points.push_back(Point(Vec2f( size.x,       size.y-ar.y), -v, null));
  if (p) { points.push_back(Point(Vec2f( size.x,      -size.y+ar.y), null, v)); }
  points.push_back(Point(Vec2f( size.x-ar.x, -size.y     ), h, null));

  return std::vector(points.begin(), points.end());
}

bool RectangleObject::is_closed() const { return true; }

}  // namespace omm
