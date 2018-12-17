#include "tags/styletag.h"

#include <QApplication>  // TODO only for icon testing
#include <QStyle>        // TODO only for icon testing
#include "properties/referenceproperty.h"

namespace omm
{

StyleTag::StyleTag()
{
  auto style_property = std::make_unique<ReferenceProperty>(AbstractPropertyOwner::Kind::Style);
  add_property( STYLE_REFERENCE_PROPERTY_KEY, std::move(style_property))
    .set_label("style").set_category("style");
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
