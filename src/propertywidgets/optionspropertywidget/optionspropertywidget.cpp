#include "propertywidgets/optionspropertywidget/optionspropertywidget.h"
#include "properties/typedproperty.h"
#include "propertywidgets/optionspropertywidget/optionsedit.h"

namespace omm
{

OptionsPropertyWidget::OptionsPropertyWidget(Scene& scene, const std::set<Property*>& properties)
  : PropertyWidget(scene, properties)
{
  auto options_edit = std::make_unique<OptionsEdit>();
  m_options_edit = options_edit.get();
  const auto get_options = std::mem_fn(&OptionsProperty::options);
  m_options_edit->set_options(Property::get_value<std::vector<std::string>, OptionsProperty>(properties, get_options));
  set_default_layout(std::move(options_edit));

  connect( m_options_edit, static_cast<void(QComboBox::*)(int)>(&OptionsEdit::currentIndexChanged),
           this, &OptionsPropertyWidget::set_properties_value );

  update_edit();
}

void OptionsPropertyWidget::update_edit()
{
  QSignalBlocker blocker(m_options_edit);
  m_options_edit->set_values(get_properties_values());
}

std::string OptionsPropertyWidget::type() const
{
  return TYPE;
}

}  // namespace omm
