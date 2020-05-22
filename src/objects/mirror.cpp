#include "objects/mirror.h"

#include <QObject>
#include "objects/empty.h"
#include "properties/optionproperty.h"
#include "properties/boolproperty.h"
#include "geometry/vec2.h"
#include "objects/path.h"
#include "scene/scene.h"

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
  options.default_style = &style;
  if (m_reflection) {
    m_reflection->draw_recursive(renderer, options);
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
  std::unique_ptr<Object> converted = std::make_unique<Empty>(scene());
  copy_properties(*converted, CopiedProperties::Compatible | CopiedProperties::User);
  copy_tags(*converted);

  if (m_reflection) {
    converted->adopt(m_reflection->clone());
  }

  return converted;
}

void Mirror::update()
{
//  m_reflection.reset();
//  const ObjectTransformation mirror_t = get_mirror_t();
//  if (is_active()) {
//    const auto n_children = this->n_children();
//    if (property(AS_PATH_PROPERTY_KEY)->value<Mode>() == Mode::Path) {
//      m_draw_children = false;
//      if (n_children == 1) {
//        Object& child = this->tree_child(0);
//        if (!!(child.flags() & Flag::IsPathLike) && !child.is_closed()) {
//          auto combined_path = std::make_unique<Path>(scene());
//          auto points = child.points();
//          for (auto& p : points) {
//            p = child.transformation().apply(p);
//          }
//          auto mirrored_points = ::transform<Point>(points, [=](const Point& p) {
//            return mirror_t.apply(p);
//          });

//          if (property(IS_INVERTED_PROPERTY_KEY)->value<bool>()) {
//            for (auto& p : mirrored_points) {
//              const auto aux = p.left_tangent;
//              p.left_tangent = p.right_tangent;
//              p.right_tangent = aux;
//            }
//            points.insert(points.end(), mirrored_points.rbegin(), mirrored_points.rend());
//          } else {
//            points.insert(points.end(), mirrored_points.begin(), mirrored_points.end());
//          }
//          combined_path->add_points(std::vector { Path::PointSequence(0, points) });
//          const auto is_closed = property(IS_CLOSED_PROPERTY_KEY)->value<bool>();
//          combined_path->property(Path::IS_CLOSED_PROPERTY_KEY)->set(is_closed);
//          m_reflection.reset(combined_path.release());
//          m_reflection->update();
//        }
//      }
//    } else {
//      m_draw_children = true;
//      if (n_children > 0) {
//        m_reflection = this->tree_children().front()->clone();
//        m_reflection->set_virtual_parent(this);
//        m_reflection->set_transformation(mirror_t.apply(m_reflection->transformation()));
//        m_reflection->update();
//      }
//    }
//  } else {
//    m_draw_children = true;
//  }
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
