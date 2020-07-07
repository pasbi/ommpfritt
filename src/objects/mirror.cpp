#include "objects/mirror.h"

#include <QObject>
#include "objects/empty.h"
#include "properties/optionproperty.h"
#include "properties/floatproperty.h"
#include "properties/boolproperty.h"
#include "geometry/vec2.h"
#include "objects/path.h"
#include "scene/scene.h"

namespace
{

using namespace omm;

Geom::Point transform_point(const Geom::Point& point, const omm::ObjectTransformation& t)
{
  const auto tp = t.apply_to_position(Vec2f{point.x(), point.y()});
  return Geom::Point{ tp.x, tp.y };
}

ObjectTransformation get_mirror_t(Mirror::Direction direction)
{
  switch (direction) {
  case Mirror::Direction::Horizontal:
    return ObjectTransformation().scaled(Vec2f(-1.0,  1.0));
  case Mirror::Direction::Vertical:
    return ObjectTransformation().scaled(Vec2f( 1.0, -1.0));
  case Mirror::Direction::Both:
    return ObjectTransformation().scaled(Vec2f( -1.0, -1.0));
  default:
    Q_UNREACHABLE();
    return ObjectTransformation();
  }
}

std::list<Geom::CubicBezier> transform_path(const ObjectTransformation& t,
                                            const Geom::Path& path, bool reverse)
{
  return ::transform<Geom::CubicBezier, std::list>(path, [t, reverse](const auto& curve) {
    const auto& cubic = dynamic_cast<const Geom::CubicBezier&>(curve);
    auto control_points = ::transform<Geom::Point>(cubic.controlPoints(), [t](auto&& point) {
      return transform_point(point, t);
    });
    if (reverse) {
      std::reverse(control_points.begin(), control_points.end());
    }
    return Geom::CubicBezier(control_points);
  });
}

}  // namespace

namespace omm
{

Mirror::Mirror(Scene* scene) : Object(scene)
{
  static const auto category = QObject::tr("Mirror");
  create_property<OptionProperty>(DIRECTION_PROPERTY_KEY)
    .set_options({ QObject::tr("Horizontal"),
                   QObject::tr("Vertical"),
                   QObject::tr("Both") })
    .set_label(QObject::tr("Direction")).set_category(category);
  auto& mode_property = create_property<OptionProperty>(AS_PATH_PROPERTY_KEY);
  mode_property.set_options({ QObject::tr("Object"),
                              QObject::tr("Path") })
    .set_label(QObject::tr("Mode")).set_category(category);
  create_property<FloatProperty>(TOLERANCE_PROPERTY_KEY)
      .set_range(0.0, std::numeric_limits<double>::max()).set_step(0.1)
      .set_label(QObject::tr("Snap tolerance")).set_category(category);
  polish();
}

Mirror::Mirror(const Mirror &other)
  : Object(other), m_reflection(other.m_reflection ? other.m_reflection->clone() : nullptr)
{
  polish();
}

void Mirror::polish()
{
  listen_to_children_changes();
  update();
}

void Mirror::draw_object(Painter &renderer, const Style& style, Painter::Options options) const
{
  assert(&renderer.scene == scene());
  if (is_active()) {
    options.default_style = &style;
    if (m_reflection) {
      m_reflection->draw_recursive(renderer, options);
    }
  } else {
    Object::draw_object(renderer, style, options);
  }
}

BoundingBox Mirror::bounding_box(const ObjectTransformation &transformation) const
{
  if (is_active() && m_reflection) {
    const ObjectTransformation t = transformation.apply(m_reflection->transformation());
    switch (property(AS_PATH_PROPERTY_KEY)->value<Mode>()) {
    case Mode::Path:
      return m_reflection->recursive_bounding_box(t);
    case Mode::Object:
      return m_reflection->recursive_bounding_box(t);
    default:
      Q_UNREACHABLE();
    }
  } else {
    return BoundingBox();
  }
}

QString Mirror::type() const { return TYPE; }

Object::ConvertedObject Mirror::convert() const
{
  if (m_draw_children) {
    std::unique_ptr<Object> converted = std::make_unique<Empty>(scene());
    auto reflection = m_reflection->clone();
    reflection->update();
    converted->adopt(std::move(reflection));
    return {std::move(converted), true};
  } else {
    return {m_reflection->clone(), false};
  }
}

Geom::PathVector Mirror::paths() const
{
  if (m_reflection && is_active()) {
    return m_reflection->geom_paths();
  } else {
    return Geom::PathVector();
  }
}

void Mirror::perform_update_object_mode()
{
  const auto n_children = this->n_children();
  if (n_children > 0) {
    const auto direction = property(DIRECTION_PROPERTY_KEY)->value<Mirror::Direction>();
    const auto make_reflection = [this](auto&& parent, Direction direction) {
      auto reflection = this->tree_children().front()->clone();
      reflection->set_virtual_parent(parent);
      reflection->set_transformation(get_mirror_t(direction).apply(reflection->transformation()));
      reflection->update();
      return reflection;
    };
    if (direction == Direction::Both) {
      m_reflection = std::make_unique<Empty>(this->scene());
      m_reflection->set_virtual_parent(this);
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

void Mirror::perform_update_path_mode()
{
  const auto n_children = this->n_children();
  if (n_children != 1) {
    m_reflection.reset();
  } else {
    Object& child = this->tree_child(0);
    const auto eps = property(TOLERANCE_PROPERTY_KEY)->value<double>();
    const auto pv = child.geom_paths();
    const bool child_is_closed = child.is_closed();
    const auto t = child.transformation();
    bool wants_to_be_closed = true;

    const auto reflect = [child_is_closed, &wants_to_be_closed, eps, t]
                         (const Geom::PathVector& pv, Mirror::Direction direction)
    {
      const auto mt = get_mirror_t(direction).apply(t);
      const auto are_close = [eps](const Geom::Point& a, const Geom::Point& b) {
        return (a - b).length() < eps;
      };
      std::vector<Geom::Path> paths;
      paths.reserve(pv.size());
      for (auto&& path : pv) {
        if (!path.empty()) {
          const std::list<Geom::CubicBezier> reflected_path = transform_path(mt, path, true);
          std::list<Geom::CubicBezier> original_path = transform_path(t, path, false);

          const bool close_ends = are_close(transform_point(path.finalPoint(), t),
                                            transform_point(path.finalPoint(), mt));
          const bool close_mids = are_close(transform_point(path.initialPoint(), t),
                                            transform_point(path.initialPoint(), mt));
          if (!child_is_closed && (!close_ends || !close_mids)) {
            wants_to_be_closed = false;
          }

          if (close_mids && !child_is_closed) {
            original_path.insert(original_path.begin(), reflected_path.rbegin(), reflected_path.rend());
            paths.emplace_back(original_path.begin(), original_path.end());
          } else if (close_ends && !child_is_closed) {
            original_path.insert(original_path.end(), reflected_path.rbegin(), reflected_path.rend());
            paths.emplace_back(original_path.begin(), original_path.end());
          } else {
            paths.emplace_back(original_path.begin(), original_path.end());
            paths.emplace_back(reflected_path.rbegin(), reflected_path.rend());
          }
        }
      }
      return Geom::PathVector(paths.begin(), paths.end());
    };

    auto reflection = std::make_unique<Path>(scene());
    if (const auto direction = property(DIRECTION_PROPERTY_KEY)->value<Mirror::Direction>();
        direction == Direction::Both)
    {
      const auto r = reflect(pv, Direction::Horizontal);
      wants_to_be_closed = !wants_to_be_closed;
      reflection->set(reflect(r, Direction::Vertical));
    } else {
      reflection->set(reflect(pv, direction));
    }

    reflection->property(Path::IS_CLOSED_PROPERTY_KEY)->set(wants_to_be_closed);
    const auto interpolation =  child.has_property(Path::INTERPOLATION_PROPERTY_KEY)
        ? child.property(Path::INTERPOLATION_PROPERTY_KEY)->value<InterpolationMode>()
        : InterpolationMode::Bezier;
    reflection->property(Path::INTERPOLATION_PROPERTY_KEY)->set(interpolation);
    m_reflection.reset(reflection.release());
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
      perform_update_path_mode();
      m_draw_children = false;
      break;
    case Mode::Object:
      perform_update_object_mode();
      m_draw_children = true;
      break;
    default:
      Q_UNREACHABLE();
    }
  } else {
    m_draw_children = true;
  }
}

void Mirror::on_property_value_changed(Property *property)
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

void Mirror::on_child_added(Object &child)
{
  Object::on_child_added(child);
  update();
}

void Mirror::on_child_removed(Object &child)
{
  Object::on_child_removed(child);
  update();
}

}  // namespace omm
