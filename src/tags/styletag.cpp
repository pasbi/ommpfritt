#include "tags/styletag.h"

#include <QApplication>  // TODO only for icon testing
#include <QStyle>        // TODO only for icon testing
#include "properties/referenceproperty.h"

namespace omm
{

StyleTag::StyleTag()
{
  add_property(STYLE_REFERENCE, std::make_unique<ReferenceProperty>())
    .set_label("style").set_category("style");
  property<ReferenceProperty>(STYLE_REFERENCE)
    .set_allowed_kinds(AbstractPropertyOwner::Kind::Style);
}

std::string StyleTag::type() const
{
  return TYPE;
}

QIcon StyleTag::icon() const
{
  return QApplication::style()->standardIcon(QStyle::SP_DialogResetButton);
}

}  // namespace omm
