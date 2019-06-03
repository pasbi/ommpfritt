#include "objects/mirror.h"

#include <QObject>
#include "objects/empty.h"
#include "properties/optionsproperty.h"
#include "properties/boolproperty.h"
#include "geometry/vec2.h"
#include "objects/path.h"

namespace omm
{

Mirror::Mirror(Scene* scene) : Object(scene)
{
  static const auto category = QObject::tr("Mirror").toStdString();
  add_property<OptionsProperty>(DIRECTION_PROPERTY_KEY)
    .set_options({ QObject::tr("Horizontal").toStdString(),
                   QObject::tr("Vertical").toStdString() })
    .set_label(QObject::tr("Direction").toStdString()).set_category(category);
  auto& mode_property = add_property<OptionsProperty>(AS_PATH_PROPERTY_KEY);
  mode_property.set_options({ QObject::tr("Object").toStdString(),
                              QObject::tr("Path").toStdString() })
    .set_label(QObject::tr("Mode").toStdString()).set_category(category);
  add_property<BoolProperty>(IS_CLOSED_PROPERTY_KEY, true)
    .set_enabled_buddy<Mode>(mode_property, { Mode::Path })
    .set_label(QObject::tr("Close").toStdString()).set_category(category);
  add_property<BoolProperty>(IS_INVERTED_PROPERTY_KEY, true)
    .set_enabled_buddy<Mode>(mode_property, { Mode::Path })
    .set_label(QObject::tr("Invert").toStdString()).set_category(category);
}

Mirror::Mirror(const Mirror &other)
  : Object(other), m_reflection(other.m_reflection ? other.m_reflection->clone() : nullptr)
{}

void Mirror::draw_object(Painter &renderer, const Style& style) const
{
  assert(&renderer.scene == scene());
  if (m_reflection) {
    m_reflection->draw_recursive(renderer, style);
  }
}

BoundingBox Mirror::bounding_box() const
{
  const auto n_children = this->n_children();
  if (is_active() && n_children > 0) {
    auto object = this->tree_children().front();
    auto bb = object->transformation().apply(object->bounding_box());
    return bb | get_mirror_t().apply(bb);
  } else {
    return BoundingBox();
  }
}

std::string Mirror::type() const { return TYPE; }
std::unique_ptr<Object> Mirror::clone() const { return std::make_unique<Mirror>(*this); }
AbstractPropertyOwner::Flag Mirror::flags() const { return Object::flags() | Flag::Convertable; }

std::unique_ptr<Object> Mirror::convert() const
{
  std::unique_ptr<Object> converted = std::make_unique<Empty>(scene());
  copy_properties(*converted);
  copy_tags(*converted);

  if (m_reflection) {
    converted->adopt(m_reflection->clone());
  }

  return converted;
}

void Mirror::update()
{
  m_reflection.reset();
  const ObjectTransformation mirror_t = get_mirror_t();
  if (is_active()) {
    const auto n_children = this->n_children();
    if (property(AS_PATH_PROPERTY_KEY)->value<Mode>() == Mode::Path) {
      m_draw_children = false;
      if (n_children == 1) {
        Object& child = this->tree_child(0);
        if (!!(child.flags() & Object::Flag::IsPathLike) && !child.is_closed()) {
          auto combined_path = std::make_unique<Path>(scene());
          auto points = child.points();
          for (auto& p : points) {
            p = child.transformation().apply(p);
          }
          auto mirrored_points = ::transform<Point>(points, [=](const Point& p) {
            return mirror_t.apply(p);
          });

          if (property(IS_INVERTED_PROPERTY_KEY)->value<bool>()) {
            for (auto& p : mirrored_points) {
              const auto aux = p.left_tangent;
              p.left_tangent = p.right_tangent;
              p.right_tangent = aux;
            }
            points.insert(points.end(), mirrored_points.rbegin(), mirrored_points.rend());
          } else {
            points.insert(points.end(), mirrored_points.begin(), mirrored_points.end());
          }
          combined_path->add_points(std::vector { Path::PointSequence(0, points) });
          const auto is_closed = property(IS_CLOSED_PROPERTY_KEY)->value<bool>();
          combined_path->property(Path::IS_CLOSED_PROPERTY_KEY)->set(is_closed);
          m_reflection.reset(combined_path.release());
          m_reflection->update();
        }
      }
    } else {
      m_draw_children = true;
      if (n_children > 0) {
        m_reflection = this->tree_children().front()->clone();
        m_reflection->set_transformation(mirror_t.apply(m_reflection->transformation()));
        m_reflection->update();
      }
    }
  } else {
    m_draw_children = true;
  }
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
