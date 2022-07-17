#include "objects/rectangleobject.h"
#include "path/edge.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"
#include "properties/floatvectorproperty.h"

namespace omm
{

RectangleObject::RectangleObject(Scene* scene) : Object(scene)
{
  static constexpr double DEFAULT_SIZE = 200.0;
  static constexpr double DEFAULT_RADIUS = 0.0;
  static constexpr double STEP = 0.01;
  static const auto category = QObject::tr("rectangle");
  create_property<FloatVectorProperty>(SIZE_PROPERTY_KEY, Vec2f(DEFAULT_SIZE, DEFAULT_SIZE))
      .set_label(QObject::tr("size"))
      .set_category(category);
  create_property<FloatVectorProperty>(RADIUS_PROPERTY_KEY, Vec2f(DEFAULT_RADIUS, DEFAULT_RADIUS))
      .set_step(Vec2f(STEP, STEP))
      .set_range(Vec2f(0.0, 0.0), Vec2f(1.0, 1.0))
      .set_label(QObject::tr("radius"))
      .set_category(category);
  create_property<FloatVectorProperty>(TENSION_PROPERTY_KEY, Vec2f(1.0, 1.0))
      .set_step(Vec2f(STEP, STEP))
      .set_range(Vec2f(0.0, 0.0), Vec2f(1.0, 1.0))
      .set_label(QObject::tr("tension"))
      .set_category(category);
  update();
}

QString RectangleObject::type() const
{
  return TYPE;
}

PathVector RectangleObject::compute_geometry() const
{
  std::deque<std::shared_ptr<PathPoint>> points;
  const auto size = property(SIZE_PROPERTY_KEY)->value<Vec2f>() / 2.0;
  const auto r = property(RADIUS_PROPERTY_KEY)->value<Vec2f>();
  const auto t = property(TENSION_PROPERTY_KEY)->value<Vec2f>();
  const Vec2f ar(size.x * r.x, size.y * r.y);

  const PolarCoordinates null(0.0, 0.0);
  const PolarCoordinates v(Vec2f(0.0, -ar.y * t.y));
  const PolarCoordinates h(Vec2f(ar.x * t.x, 0.0));
  PathVector path_vector;
  auto add = [&points, &path_vector](const auto& pos, const auto& fwd, const auto& bwd) {
    points.emplace_back(std::make_shared<PathPoint>(Point(pos, fwd, bwd), &path_vector));
  };

  static constexpr auto eps = 0.00001;
  const bool p = ar.euclidean_norm2() > eps * eps;
  if (p) {
    add(Vec2f(-size.x + ar.x, -size.y), null, -h);
  }
  add(Vec2f(-size.x, -size.y + ar.y), v, null);
  if (p) {
    add(Vec2f(-size.x, size.y - ar.y), null, -v);
  }
  add(Vec2f(-size.x + ar.x, size.y), -h, null);
  if (p) {
    add(Vec2f(size.x - ar.x, size.y), null, h);
  }
  add(Vec2f(size.x, size.y - ar.y), -v, null);
  if (p) {
    add(Vec2f(size.x, -size.y + ar.y), null, v);
  }
  add(Vec2f(size.x - ar.x, -size.y), h, null);

  points.emplace_back(points.front());

  auto& path = path_vector.add_path();
  for (std::size_t i = 1; i < points.size(); ++i) {
    path.add_edge(std::make_unique<Edge>(points.at(i - 1), points.at(i), &path));
  }
  return path_vector;
}

void RectangleObject::on_property_value_changed(Property* property)
{
  if (property == this->property(SIZE_PROPERTY_KEY)
      || property == this->property(RADIUS_PROPERTY_KEY)
      || property == this->property(TENSION_PROPERTY_KEY)) {
    update();
  } else {
    Object::on_property_value_changed(property);
  }
}

}  // namespace omm
