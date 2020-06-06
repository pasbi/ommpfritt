#include "objects/mirror.h"

#include <QObject>
#include "objects/empty.h"
#include "properties/optionproperty.h"
#include "properties/boolproperty.h"
#include "geometry/vec2.h"
#include "objects/path.h"
#include "scene/scene.h"

namespace
{

Geom::Point transform_point(const Geom::Point& point, const omm::ObjectTransformation& t)
{
  const auto tp = t.apply_to_position(omm::Vec2f{point.x(), point.y()});
  return Geom::Point{ tp.x, tp.y };
}

}  // namespace

namespace omm
{

Mirror::Mirror(Scene* scene) : Object(scene)
{
  static const auto category = QObject::tr("Mirror");
  create_property<OptionProperty>(DIRECTION_PROPERTY_KEY)
    .set_options({ QObject::tr("Horizontal"),
                   QObject::tr("Vertical") })
    .set_label(QObject::tr("Direction")).set_category(category);
  auto& mode_property = create_property<OptionProperty>(AS_PATH_PROPERTY_KEY);
  mode_property.set_options({ QObject::tr("Object"),
                              QObject::tr("Path") })
    .set_label(QObject::tr("Mode")).set_category(category);
  create_property<BoolProperty>(IS_CLOSED_PROPERTY_KEY, true)
    .set_label(QObject::tr("Close")).set_category(category);
  create_property<BoolProperty>(IS_INVERTED_PROPERTY_KEY, true)
    .set_label(QObject::tr("Invert")).set_category(category);
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
Flag Mirror::flags() const { return Object::flags() | Flag::Convertible; }

std::unique_ptr<Object> Mirror::convert() const
{
  if (m_draw_children) {
    std::unique_ptr<Object> converted = std::make_unique<Empty>(scene());
    converted->adopt(m_reflection->clone());
    return converted;
  } else {
    return m_reflection->clone();
  }
}

Geom::PathVector Mirror::paths() const
{
  if (m_reflection && is_active()) {
    return m_reflection->paths();
  } else {
    return Geom::PathVector();
  }
}

void Mirror::perform_update_object_mode()
{
  const ObjectTransformation mirror_t = get_mirror_t();
  const auto n_children = this->n_children();
  if (n_children > 0) {
    m_reflection = this->tree_children().front()->clone();
    m_reflection->set_virtual_parent(this);
    m_reflection->set_transformation(mirror_t.apply(m_reflection->transformation()));
    m_reflection->update();
  }
}

void Mirror::perform_update_path_mode()
{
  const auto n_children = this->n_children();
  if (n_children != 1) {
    m_reflection.reset();
  } else {
    Object& child = this->tree_child(0);
    const auto transform_path = [](const ObjectTransformation& t, const auto& path, bool reverse) {
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
    };

    const auto are_close = [](const Geom::Point& a, const Geom::Point& b) {
      return (a - b).length() < 10;
    };

    const auto child_paths = child.paths();
    std::vector<Geom::Path> paths;
    paths.reserve(child_paths.size());
    std::vector<bool> wants_to_be_closed;
    wants_to_be_closed.reserve(child_paths.size());
    const bool child_is_closed = child.is_closed();

    const auto t = child.transformation();
    const auto mt = get_mirror_t().apply(t);
    for (auto&& path : child_paths) {
      if (!path.empty()) {
        const std::list<Geom::CubicBezier> reflected_path = transform_path(mt, path, true);
        std::list<Geom::CubicBezier> original_path = transform_path(t, path, false);

        const bool close_ends = are_close(transform_point(path.finalPoint(), t),
                                          transform_point(path.finalPoint(), mt));
        const bool close_mids = are_close(transform_point(path.initialPoint(), t),
                                          transform_point(path.initialPoint(), mt));
        wants_to_be_closed.push_back(child_is_closed || (close_ends && close_mids));
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
    auto reflection = std::make_unique<Path>(scene());
    reflection->set(Geom::PathVector(paths.begin(), paths.end()));

    reflection->property(Path::IS_CLOSED_PROPERTY_KEY)->set(std::all_of(wants_to_be_closed.begin(),
                                                                        wants_to_be_closed.end(),
                                                                        ::identity));
    m_reflection.reset(reflection.release());
  }
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
  if (   property == this->property(DIRECTION_PROPERTY_KEY)
      || property == this->property(IS_CLOSED_PROPERTY_KEY)
      || property == this->property(IS_INVERTED_PROPERTY_KEY))
  {
    update();
  } else if (property == this->property(AS_PATH_PROPERTY_KEY)) {
    this->property(IS_CLOSED_PROPERTY_KEY)->set_visible(property->value<Mode>() == Mode::Path);
    this->property(IS_INVERTED_PROPERTY_KEY)->set_visible(property->value<Mode>() == Mode::Path);
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

ObjectTransformation Mirror::get_mirror_t() const
{
  switch (property(DIRECTION_PROPERTY_KEY)->value<Mirror::Direction>()) {
  case omm::Mirror::Direction::Horizontal:
    return omm::ObjectTransformation().scaled(omm::Vec2f(-1.0,  1.0));
  case omm::Mirror::Direction::Vertical:
    return omm::ObjectTransformation().scaled(omm::Vec2f( 1.0, -1.0));
  }
  Q_UNREACHABLE();
}

}  // namespace omm
