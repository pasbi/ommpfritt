#include "propertywidgets/boolpropertywidget/boolpropertywidget.h"

#include "propertywidgets/multivalueedit.h"
#include "properties/typedproperty.h"
#include "propertywidgets/boolpropertywidget/checkbox.h"


namespace omm
{

BoolPropertyWidget::BoolPropertyWidget(Scene& scene, const Property::SetOfProperties& properties)
  : PropertyWidget(scene, properties)
{
  auto checkbox = std::make_unique<CheckBox>();
  m_checkbox = checkbox.get();
  set_default_layout(std::move(checkbox));

  connect(m_checkbox, &QCheckBox::clicked, [this](const bool checked) {
    set_properties_value(checked);
  });

  on_property_value_changed();
}

void BoolPropertyWidget::on_property_value_changed()
{
  QSignalBlocker blocker(m_checkbox);
  m_checkbox->set_values(get_properties_values());
}

std::string BoolPropertyWidget::type() const
{
  return "BoolPropertyWidget";
}

}  // namespace omm
