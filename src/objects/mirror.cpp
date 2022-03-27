#include "objects/mirror.h"

#include "geometry/vec2.h"
#include "objects/empty.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"
#include "path/lib2geomadapter.h"
#include "objects/pathobject.h"
#include "properties/boolproperty.h"
#include "properties/floatproperty.h"
#include "properties/optionproperty.h"
#include "renderers/painter.h"
#include "renderers/painteroptions.h"
#include "scene/disjointpathpointsetforest.h"
#include "scene/scene.h"
#include <QObject>
#include "path/path.h"
#include "geometry/objecttransformation.h"

namespace
{

using namespace omm;

Path& make_reflection(PathVector& pv, const Path& original, const Mirror::Direction direction, const double eps)
{
  auto& path = pv.add_path(std::make_unique<Path>(original, &pv));
  const auto s = Vec2f{direction == Mirror::Direction::Horizontal ? -1.0 : 1.0,
                       direction == Mirror::Direction::Vertical ? -1.0 : 1.0};
  const auto transform = ObjectTransformation{}.scaled(s);
  for (auto* p : path.points()) {
    p->set_geometry(transform.apply(p->geometry()));
  }

  const auto join_if_close = [&pv, eps2 = eps * eps](PathPoint& p1, PathPoint& p2) {
    if ((p1.geometry().position() - p2.geometry().position()).euclidean_norm2() < eps2) {
      pv.joined_points().insert({&p1, &p2});
      auto g1 = p1.geometry();
      auto g2 = p2.geometry();
      const auto p = (g1.position() + g2.position()) / 2.0;
      g1.set_position(p);
      p1.set_geometry(g1);

      g2.set_position(p);
      p2.set_geometry(g2);
    }
  };

  if (const auto n = path.size(); n > 1) {
    join_if_close(path.at(0), original.at(0));
    join_if_close(path.at(n - 1), original.at(n - 1));
  }
  return path;
}


ObjectTransformation get_mirror_t(Mirror::Direction direction)
{
  switch (direction) {
  case Mirror::Direction::Horizontal:
    return ObjectTransformation().scaled(Vec2f(-1.0, 1.0));
  case Mirror::Direction::Vertical:
    return ObjectTransformation().scaled(Vec2f(1.0, -1.0));
  case Mirror::Direction::Both:
    return ObjectTransformation().scaled(Vec2f(-1.0, -1.0));
  default:
    Q_UNREACHABLE();
    return ObjectTransformation();
  }
}

}  // namespace

namespace omm
{

Mirror::Mirror(Scene* scene) : Object(scene)
{
  static constexpr double TOLERANCE_STEP = 0.1;
  static const auto category = QObject::tr("Mirror");
  create_property<OptionProperty>(DIRECTION_PROPERTY_KEY)
      .set_options({QObject::tr("Horizontal"), QObject::tr("Vertical"), QObject::tr("Both")})
      .set_label(QObject::tr("Direction"))
      .set_category(category);
  auto& mode_property = create_property<OptionProperty>(AS_PATH_PROPERTY_KEY);
  mode_property.set_options({QObject::tr("Object"), QObject::tr("Path")})
      .set_label(QObject::tr("Mode"))
      .set_category(category);
  create_property<FloatProperty>(TOLERANCE_PROPERTY_KEY)
      .set_range(0.0, std::numeric_limits<double>::max())
      .set_step(TOLERANCE_STEP)
      .set_label(QObject::tr("Snap tolerance"))
      .set_category(category);
  polish();
}

Mirror::Mirror(const Mirror& other)
    : Object(other), m_reflection(other.m_reflection ? other.m_reflection->clone() : nullptr)
{
  polish();
}

void Mirror::polish()
{
  listen_to_children_changes();
  Mirror::update();
}

void Mirror::draw_object(Painter& renderer,
                         const Style& style,
                         const PainterOptions& options) const
{
  assert(&renderer.scene == scene());
  if (is_active()) {
    auto options_copy = options;
    options_copy.default_style = &style;
    if (m_reflection) {
      m_reflection->draw_recursive(renderer, options_copy);
    }
  } else {
    Object::draw_object(renderer, style, options);
  }
}

BoundingBox Mirror::bounding_box(const ObjectTransformation& transformation) const
{
  if (is_active() && m_reflection) {
    const ObjectTransformation t = transformation.apply(m_reflection->transformation());
    switch (property(AS_PATH_PROPERTY_KEY)->value<Mode>()) {
    case Mode::Path:
      [[fallthrough]];
    case Mode::Object:
      return m_reflection->recursive_bounding_box(t);
    default:
      Q_UNREACHABLE();
    }
  } else {
    return BoundingBox{};
  }
}

QString Mirror::type() const
{
  return TYPE;
}

std::unique_ptr<Object> Mirror::convert(bool& keep_children) const
{
  if (m_draw_children) {
    std::unique_ptr<Object> converted = std::make_unique<Empty>(scene());
    auto reflection = m_reflection->clone();
    reflection->update();
    converted->adopt(std::move(reflection));
    keep_children = true;
    return converted;
  } else {
    keep_children = false;
    auto clone = m_reflection->clone();
    return clone;
  }
}

PathVector Mirror::compute_path_vector() const
{
  if (!is_active()) {
    return {};
  }
  switch (property(AS_PATH_PROPERTY_KEY)->value<Mode>()) {
  case Mode::Path:
    return type_cast<PathObject&>(*m_reflection).geometry();
  case Mode::Object:
    return PathVector{m_reflection->path_vector(), nullptr};
  default:
    Q_UNREACHABLE();
  }
}

void Mirror::update_object_mode()
{
  const auto n_children = this->n_children();
  if (n_children > 0) {
    const auto direction = property(DIRECTION_PROPERTY_KEY)->value<Mirror::Direction>();
    const auto make_reflection = [this](auto* const parent, const Direction direction) {
      auto reflection = this->tree_children().front()->clone();
      reflection->set_virtual_parent(parent);
      reflection->set_transformation(get_mirror_t(direction).apply(reflection->transformation()));
      reflection->update();
      return reflection;
    };
    if (direction == Direction::Both) {
      m_reflection = std::make_unique<Empty>(this->scene());
      m_reflection->adopt(make_reflection(m_reflection.get(), Direction::Horizontal));
      m_reflection->adopt(make_reflection(m_reflection.get(), Direction::Vertical));
      m_reflection->adopt(make_reflection(m_reflection.get(), Direction::Both));
    } else {
      m_reflection = make_reflection(this, direction);
    }
  } else {
    m_reflection.reset();
  }
}

void Mirror::update_path_mode()
{
  const auto n_children = this->n_children();
  if (n_children != 1) {
    m_reflection.reset();
  } else {
    const auto eps = property(TOLERANCE_PROPERTY_KEY)->value<double>();
    Object& child = this->tree_child(0);
    auto reflection = std::make_unique<PathObject>(scene());
    reflection->geometry().unshare_joined_points(std::make_unique<DisjointPathPointSetForest>());
    assert(!reflection->geometry().joined_points_shared());
    auto& pv = reflection->geometry();
    for (const auto* const path : child.path_vector().paths()) {
      auto& original = pv.add_path(std::make_unique<Path>(*path, &pv));
      if (const auto direction = property(DIRECTION_PROPERTY_KEY)->value<Mirror::Direction>();
          direction == Direction::Both)
      {
        auto& reflection = make_reflection(pv, original, Direction::Horizontal, eps);
        make_reflection(pv, original, Direction::Vertical, eps);
        make_reflection(pv, reflection, Direction::Vertical, eps);
      } else {
        make_reflection(pv, original, direction, eps);
      }
    }
    const auto interpolation = child.has_property(PathObject::INTERPOLATION_PROPERTY_KEY)
                             ? child.property(PathObject::INTERPOLATION_PROPERTY_KEY)->value<InterpolationMode>()
                             : InterpolationMode::Bezier;
    reflection->property(PathObject::INTERPOLATION_PROPERTY_KEY)->set(interpolation);

    m_reflection = std::move(reflection);
  }
}

void Mirror::update_property_visibility()
{
  const auto mode = property(AS_PATH_PROPERTY_KEY)->value<Mode>();
  property(TOLERANCE_PROPERTY_KEY)->set_enabledness(mode == Mode::Path);
}

void Mirror::update()
{
  if (is_active()) {
    switch (property(AS_PATH_PROPERTY_KEY)->value<Mode>()) {
    case Mode::Path:
      update_path_mode();
      m_draw_children = false;
      break;
    case Mode::Object:
      update_object_mode();
      m_draw_children = true;
      break;
    default:
      Q_UNREACHABLE();
    }
  } else {
    m_draw_children = true;
  }
  Object::update();
}

void Mirror::on_property_value_changed(Property* property)
{
  if (pmatch(property, {DIRECTION_PROPERTY_KEY, TOLERANCE_PROPERTY_KEY})) {
    update();
  } else if (pmatch(property, {AS_PATH_PROPERTY_KEY})) {
    update_property_visibility();
    update();
  } else {
    Object::on_property_value_changed(property);
  }
}

void Mirror::on_child_added(Object& child)
{
  Object::on_child_added(child);
  update();
}

void Mirror::on_child_removed(Object& child)
{
  Object::on_child_removed(child);
  update();
}

}  // namespace omm
