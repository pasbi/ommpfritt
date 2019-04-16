#include "objects/mirror.h"

#include <QObject>
#include "objects/empty.h"
#include "properties/optionsproperty.h"
#include "geometry/vec2.h"


namespace omm
{

Mirror::Mirror(Scene* scene) : Object(scene)
{
  static const auto category = QObject::tr("Mirror").toStdString();
  add_property<OptionsProperty>(DIRECTION_PROPERTY_KEY)
    .set_options({ QObject::tr("Horizontal").toStdString(),
                   QObject::tr("Vertical").toStdString() })
      .set_label(QObject::tr("Direction").toStdString()).set_category(category);
}

Mirror::Mirror(const Mirror &other)
  : Object(other), m_reflection(other.m_reflection->clone())
{}


void Mirror::draw_object(AbstractRenderer& renderer, const Style& style) const
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
    auto object = this->children().front();
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

  for (auto&& child : this->children()) {
    converted->adopt(child->clone());
  }

  return converted;
}

void Mirror::update()
{
  const auto n_children = this->n_children();
  if (is_active() && n_children > 0) {
    m_reflection = this->children().front()->clone();
    m_reflection->set_transformation(get_mirror_t().apply(m_reflection->transformation()));
  } else {
    m_reflection.reset();
  }
}

ObjectTransformation Mirror::get_mirror_t() const
{
  switch (property(DIRECTION_PROPERTY_KEY).value<Mirror::Direction>()) {
  case omm::Mirror::Direction::Horizontal:
    return omm::ObjectTransformation().scaled(omm::Vec2f(-1.0,  1.0));
  case omm::Mirror::Direction::Vertical:
    return omm::ObjectTransformation().scaled(omm::Vec2f( 1.0, -1.0));
  }
  Q_UNREACHABLE();
}

}  // namespace omm
