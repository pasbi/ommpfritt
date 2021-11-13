#include "objects/mirror.h"

#include "geometry/vec2.h"
#include "objects/empty.h"
#include "objects/path.h"
#include "properties/boolproperty.h"
#include "properties/floatproperty.h"
#include "properties/optionproperty.h"
#include "renderers/painter.h"
#include "renderers/painteroptions.h"
#include "scene/scene.h"
#include <QObject>

namespace
{
using namespace omm;

Geom::Point transform_point(const Geom::Point& point, const omm::ObjectTransformation& t)
{
  const auto tp = t.apply_to_position(Vec2f{point.x(), point.y()});
  return Geom::Point{tp.x, tp.y};
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

std::list<Geom::CubicBezier>
transform_path(const ObjectTransformation& t, const Geom::Path& path, bool reverse)
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

Geom::PathVector reflect(bool& wants_to_be_closed,
                         const Geom::PathVector& pv,
                         const Mirror::Direction direction,
                         const bool child_is_closed,
                         const double eps)
{
  const auto mt = get_mirror_t(direction);
  const auto are_close = [eps](const Geom::Point& a, const Geom::Point& b) {
    return (a - b).length() < eps;
  };
  std::vector<Geom::Path> paths;
  paths.reserve(pv.size());
  for (const auto& path : pv) {
    if (!path.empty()) {
      const std::list<Geom::CubicBezier> reflected_path = transform_path(mt, path, true);
      std::list<Geom::CubicBezier> original_path = transform_path(ObjectTransformation{}, path, false);

      const bool close_ends = are_close(path.finalPoint(),  transform_point(path.finalPoint(), mt));
      const bool close_mids = are_close(path.initialPoint(), transform_point(path.initialPoint(), mt));
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
  return {paths.begin(), paths.end()};
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
    return BoundingBox();
  }
}

QString Mirror::type() const
{
  return TYPE;
}

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
    return {};
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

Geom::PathVector transform(const Geom::PathVector& pv, const ObjectTransformation& t)
{
  auto transformed_pv = ::transform<Geom::Path, std::vector>(pv, [&t](const auto& p) {
    const auto cubics = transform_path(t, p, false);
    return Geom::Path{cubics.begin(), cubics.end()};
  });
  return {std::move_iterator{transformed_pv.begin()}, std::move_iterator{transformed_pv.end()}};
}

void Mirror::perform_update_path_mode()
{
  const auto n_children = this->n_children();
  if (n_children != 1) {
    m_reflection.reset();
  } else {
    Object& child = this->tree_child(0);
    const auto pv = child.geom_paths();
    bool wants_to_be_closed = true;

    const bool child_is_closed = child.is_closed();
    const auto eps = property(TOLERANCE_PROPERTY_KEY)->value<double>();
    auto reflection = std::make_unique<Path>(scene());
    if (const auto direction = property(DIRECTION_PROPERTY_KEY)->value<Mirror::Direction>();
        direction == Direction::Both)
    {
      auto r = omm::transform(pv, child.transformation());
      r = reflect(wants_to_be_closed, r, Direction::Horizontal, child_is_closed, eps);
      wants_to_be_closed = !wants_to_be_closed;
      r = reflect(wants_to_be_closed, r, Direction::Vertical, child_is_closed, eps);
      reflection->set(r);
    } else {
      auto r = omm::transform(pv, child.transformation());
      r = reflect(wants_to_be_closed, r, direction, child_is_closed, eps);
      reflection->set(r);
    }

    reflection->property(Path::IS_CLOSED_PROPERTY_KEY)->set(wants_to_be_closed);
    const auto interpolation = child.has_property(Path::INTERPOLATION_PROPERTY_KEY)
                             ? child.property(Path::INTERPOLATION_PROPERTY_KEY)->value<InterpolationMode>()
                             : InterpolationMode::Bezier;
    reflection->property(Path::INTERPOLATION_PROPERTY_KEY)->set(interpolation);
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
