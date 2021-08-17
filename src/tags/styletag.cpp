#include "tags/styletag.h"

#include "main/application.h"
#include "managers/propertymanager/propertymanager.h"
#include "objects/object.h"
#include "properties/referenceproperty.h"
#include "properties/triggerproperty.h"
#include "renderers/style.h"
#include "scene/mailbox.h"
#include "scene/scene.h"

namespace omm
{
StyleTag::StyleTag(Object& owner) : Tag(owner)
{
  const QString category = QObject::tr("Basic");
  create_property<ReferenceProperty>(STYLE_REFERENCE_PROPERTY_KEY)
      .set_filter(PropertyFilter({Kind::Style}, {{}}))
      .set_label(QObject::tr("style"))
      .set_category(category);

  create_property<TriggerProperty>(EDIT_STYLE_PROPERTY_KEY)
      .set_label(QObject::tr("Edit style ..."))
      .set_category(category);
}

QString StyleTag::type() const
{
  return TYPE;
}
void StyleTag::evaluate()
{
}
Flag StyleTag::flags() const
{
  return Tag::flags();
}

void StyleTag::on_property_value_changed(Property* property)
{
  // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
  if (property == this->property(STYLE_REFERENCE_PROPERTY_KEY)) {
    Q_EMIT owner->scene()->mail_box().object_appearance_changed(*owner);
  } else if (property == this->property(EDIT_STYLE_PROPERTY_KEY)) {
    auto* style = this->property(STYLE_REFERENCE_PROPERTY_KEY)->value<AbstractPropertyOwner*>();
    if (style != nullptr) {
      Manager& manager = Application::instance().get_active_manager(PropertyManager::TYPE);

      // if manager is the PropertyManager hosting the button which has been pressed, it must
      // not be changed now.
      QTimer::singleShot(0, &manager, [&manager, style]() {
        dynamic_cast<PropertyManager&>(manager).set_selection({style});
      });
    }
  }
}

}  // namespace omm
