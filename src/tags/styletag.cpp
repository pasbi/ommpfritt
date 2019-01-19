#include "tags/styletag.h"

#include <QApplication>  // TODO only for icon testing
#include <QStyle>        // TODO only for icon testing
#include "properties/referenceproperty.h"

namespace omm
{

StyleTag::StyleTag(Object& owner)
  : Tag(owner)
{
  add_property<ReferenceProperty>(STYLE_REFERENCE_PROPERTY_KEY)
    .set_allowed_kinds(AbstractPropertyOwner::Kind::Style)
    .set_label("style").set_category("style");
}


QIcon StyleTag::icon() const
{
  return QApplication::style()->standardIcon(QStyle::SP_DialogResetButton);
}

std::string StyleTag::type() const { return TYPE; }
std::unique_ptr<Tag> StyleTag::clone() const { return std::make_unique<StyleTag>(*this); }
void StyleTag::evaluate() {}

}  // namespace omm
