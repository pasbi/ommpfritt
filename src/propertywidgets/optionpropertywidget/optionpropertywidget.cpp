#include "propertywidgets/optionpropertywidget/optionpropertywidget.h"
#include "properties/typedproperty.h"
#include "propertywidgets/optionpropertywidget/optionsedit.h"

#include <QLabel>
#include <QVBoxLayout>

namespace omm
{
OptionPropertyWidget::OptionPropertyWidget(Scene& scene, const std::set<Property*>& properties)
    : PropertyWidget(scene, properties)
{
  auto options_edit = std::make_unique<OptionsEdit>();
  connect(options_edit.get(),
          static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this,
          [this](int value) { set_properties_value(value); });
  m_options_edit = options_edit.get();
  const auto get_options = std::mem_fn(&OptionProperty::options);

  QSignalBlocker blocker(m_options_edit);
  m_options_edit->set_options(Property::get_value<std::deque<QString>, OptionProperty>(properties, get_options));
  m_options_edit->prefix = label();
  set_widget(std::move(options_edit));
  OptionPropertyWidget::update_edit();
}

void OptionPropertyWidget::update_edit()
{
  QSignalBlocker blocker(m_options_edit);
  m_options_edit->set_values(get_properties_values());
}

}  // namespace omm
