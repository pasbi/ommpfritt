#include "propertywidgets/boolpropertywidget/boolpropertywidget.h"

#include "propertywidgets/multivalueedit.h"
#include "properties/typedproperty.h"
#include "propertywidgets/boolpropertywidget/checkbox.h"

namespace omm
{

BoolPropertyWidget::BoolPropertyWidget(Scene& scene, const std::set<Property*>& properties)
  : PropertyWidget(scene, properties)
{
  auto checkbox = std::make_unique<CheckBox>();
  m_checkbox = checkbox.get();
  connect(m_checkbox, &QAbstractButton::clicked, [this]() {
    set_properties_value(m_checkbox->checkState() == Qt::Checked);
  });
  set_default_layout(std::move(checkbox));

  update_edit();
}

void BoolPropertyWidget::update_edit()
{
  QSignalBlocker blocker(m_checkbox);
  m_checkbox->set_values(get_properties_values());
}

QString BoolPropertyWidget::type() const
{
  return TYPE;
}

}  // namespace omm
