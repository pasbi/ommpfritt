#include "objects/outline.h"

#include <QObject>
#include "objects/empty.h"
#include "properties/floatproperty.h"
#include "properties/referenceproperty.h"
#include "geometry/vec2.h"


namespace omm
{

Outline::Outline(Scene* scene) : Object(scene)
{
  static const auto category = QObject::tr("Outline").toStdString();
  add_property<FloatProperty>(OFFSET_PROPERTY_KEY)
    .set_label(QObject::tr("Offset").toStdString()).set_category(category);
  add_property<ReferenceProperty>(REFERENCE_PROPERTY_KEY)
    .set_allowed_kinds(AbstractPropertyOwner::Kind::Object)
      .set_label(QObject::tr("Offset").toStdString()).set_category(category);
}

Outline::Outline(const Outline &other)
  : Object(other), m_outline(other.m_outline->clone())
{ }

void Outline::draw_object(AbstractRenderer& renderer, const Style& style) const
{
  assert(&renderer.scene == scene());
  if (m_outline) {
    m_outline->draw_recursive(renderer, style);
  }
}

BoundingBox Outline::bounding_box() const
{
  return m_outline ? m_outline->bounding_box() : BoundingBox();
}

std::string Outline::type() const { return TYPE; }
std::unique_ptr<Object> Outline::clone() const { return std::make_unique<Outline>(*this); }
AbstractPropertyOwner::Flag Outline::flags() const { return Object::flags() | Flag::Convertable; }

void Outline::update()
{
  if (is_active()) {
    auto* ref = property(REFERENCE_PROPERTY_KEY).value<AbstractPropertyOwner*>();
    const auto t = property(OFFSET_PROPERTY_KEY).value<double>();
    auto* object = ref ? ref->cast<Object>() : nullptr;
    if (object) {
      m_outline = object->outline(t);
    } else {
      m_outline.reset();
    }
  }
}

std::unique_ptr<Object> Outline::convert() const
{
  return m_outline->clone();
}


}  // namespace omm
