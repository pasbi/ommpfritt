#include "propertywidgets/triggerpropertywidget/triggerpropertywidget.h"

#include "propertywidgets/multivalueedit.h"
#include "properties/typedproperty.h"
#include <QPushButton>


namespace omm
{

TriggerPropertyWidget::TriggerPropertyWidget( Scene& scene, const std::string& key,
                                              const std::set<Property*>& properties )
  : PropertyWidget(scene, key, properties)
{
  auto button = std::make_unique<QPushButton>();
  connect(button.get(), &QPushButton::clicked, this, &TriggerPropertyWidget::trigger);
  set_default_layout(std::move(button));
}

void TriggerPropertyWidget::trigger()
{
  for (Property* p : properties()) {
    static_cast<TriggerProperty&>(*p).trigger();
  }
}

std::string TriggerPropertyWidget::type() const { return TYPE; }
void TriggerPropertyWidget::update_edit() {}

}  // namespace omm
