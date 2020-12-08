#include "propertywidgets/triggerpropertywidget/triggerpropertywidget.h"

#include "properties/typedproperty.h"
#include "propertywidgets/multivalueedit.h"
#include <QPushButton>

namespace omm
{
TriggerPropertyWidget::TriggerPropertyWidget(Scene& scene, const std::set<Property*>& properties)
    : PropertyWidget(scene, properties)
{
  auto button = std::make_unique<QPushButton>(label());
  connect(button.get(), &QPushButton::clicked, this, &TriggerPropertyWidget::trigger);
  set_widget(std::move(button));
}

void TriggerPropertyWidget::trigger()
{
  for (Property* p : properties()) {
    dynamic_cast<TriggerProperty&>(*p).trigger();
  }
}

void TriggerPropertyWidget::update_edit()
{
}

}  // namespace omm
