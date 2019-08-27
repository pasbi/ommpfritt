#include "tags/styletag.h"

#include <QApplication>  // TODO only for icon testing
#include <QStyle>        // TODO only for icon testing
#include "properties/referenceproperty.h"
#include "renderers/style.h"
#include "objects/object.h"

namespace omm
{

StyleTag::StyleTag(Object& owner)
  : Tag(owner)
{
  create_property<ReferenceProperty>(STYLE_REFERENCE_PROPERTY_KEY)
    .set_allowed_kinds(Kind::Style)
    .set_label(QObject::tr("style").toStdString())
    .set_category(QObject::tr("style").toStdString());
}

QIcon StyleTag::icon() const
{
  const auto* style = property(STYLE_REFERENCE_PROPERTY_KEY)->value<AbstractPropertyOwner*>();
  if (style == nullptr) {
    return QApplication::style()->standardIcon(QStyle::SP_DialogResetButton);
  } else {
    assert(style->kind() == Kind::Style);
    return static_cast<const Style*>(style)->icon();
  }
}

std::string StyleTag::type() const { return TYPE; }
std::unique_ptr<Tag> StyleTag::clone() const { return std::make_unique<StyleTag>(*this); }
void StyleTag::evaluate() {}
AbstractPropertyOwner::Flag StyleTag::flags() const { return Tag::flags(); }

void StyleTag::on_property_value_changed(Property *property)
{
  if (property == this->property(STYLE_REFERENCE_PROPERTY_KEY)) {
    Q_EMIT owner->appearance_changed(owner);
  }
}

}  // namespace omm
