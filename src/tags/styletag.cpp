#include "tags/styletag.h"

#include "properties/referenceproperty.h"
#include "renderers/style.h"
#include "objects/object.h"
#include "scene/scene.h"
#include "scene/messagebox.h"

namespace omm
{

StyleTag::StyleTag(Object& owner)
  : Tag(owner)
{
  create_property<ReferenceProperty>(STYLE_REFERENCE_PROPERTY_KEY)
    .set_allowed_kinds(Kind::Style)
    .set_label(QObject::tr("style"))
    .set_category(QObject::tr("style"));
}

QString StyleTag::type() const { return TYPE; }
std::unique_ptr<Tag> StyleTag::clone() const { return std::make_unique<StyleTag>(*this); }
void StyleTag::evaluate() {}
AbstractPropertyOwner::Flag StyleTag::flags() const { return Tag::flags(); }

void StyleTag::on_property_value_changed(Property *property)
{
  if (property == this->property(STYLE_REFERENCE_PROPERTY_KEY)) {
    owner->scene()->message_box().appearance_changed(*owner);
  }
}

}  // namespace omm
