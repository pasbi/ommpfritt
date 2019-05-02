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
  connect(checkbox.get(), &QCheckBox::stateChanged, [this](int state) {
    switch (state) {
    case Qt::Checked:
      set_properties_value(true);
      break;
    case Qt::Unchecked:
      [[fallthrough]];
    case Qt::PartiallyChecked:
      set_properties_value(false);
      break;
    }
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
