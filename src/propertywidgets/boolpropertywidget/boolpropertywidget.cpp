#include "propertywidgets/boolpropertywidget/boolpropertywidget.h"

#include "propertywidgets/multivalueedit.h"
#include "properties/typedproperty.h"
#include "propertywidgets/boolpropertywidget/checkbox.h"

namespace omm
{

BoolPropertyWidget::BoolPropertyWidget( Scene& scene, const std::string& key,
                                        const std::set<Property*>& properties )
  : PropertyWidget(scene, key, properties)
{
  auto checkbox = std::make_unique<CheckBox>([this](const bool& value) {
    set_properties_value(value);
  });
  m_checkbox = checkbox.get();
  set_default_layout(std::move(checkbox));

  update_edit();
}

void BoolPropertyWidget::update_edit()
{
  QSignalBlocker blocker(m_checkbox);
  m_checkbox->set_values(get_properties_values());
}

std::string BoolPropertyWidget::type() const
{
  return TYPE;
}

}  // namespace omm
