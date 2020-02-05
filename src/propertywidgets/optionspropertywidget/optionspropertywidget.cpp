#include "propertywidgets/optionspropertywidget/optionspropertywidget.h"
#include "properties/typedproperty.h"
#include "propertywidgets/optionspropertywidget/optionsedit.h"

#include <QLabel>
#include <QVBoxLayout>

namespace omm
{

OptionsPropertyWidget::OptionsPropertyWidget(Scene& scene, const std::set<Property*>& properties)
  : PropertyWidget(scene, properties)
{
  auto options_edit = std::make_unique<OptionsEdit>();
  connect(options_edit.get(), static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          [this](int value) { set_properties_value(value); });
  m_options_edit = options_edit.get();
  const auto get_options = std::mem_fn(&OptionsProperty::options);

  QSignalBlocker blocker(m_options_edit);
  m_options_edit->set_options(
    Property::get_value<std::vector<QString>, OptionsProperty>(properties, get_options));
  m_options_edit->prefix = label();
  set_widget(std::move(options_edit));
  update_edit();
}

void OptionsPropertyWidget::update_edit()
{
  QSignalBlocker blocker(m_options_edit);
  m_options_edit->set_values(get_properties_values());
}

QString OptionsPropertyWidget::type() const
{
  return TYPE;
}

}  // namespace omm
