#include "tags/styletag.h"

#include <QApplication>  // TODO only for icon testing
#include <QStyle>        // TODO only for icon testing
#include "properties/referenceproperty.h"
#include "renderers/style.h"

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
  const auto* style = property(STYLE_REFERENCE_PROPERTY_KEY).value<AbstractPropertyOwner*>();
  if (style == nullptr) {
    return QApplication::style()->standardIcon(QStyle::SP_DialogResetButton);
  } else {
    assert(style->kind() == AbstractPropertyOwner::Kind::Style);
    return static_cast<const Style*>(style)->icon();
  }
}

std::string StyleTag::type() const { return TYPE; }
std::unique_ptr<Tag> StyleTag::clone() const { return std::make_unique<StyleTag>(*this); }
void StyleTag::evaluate() {}

}  // namespace omm
