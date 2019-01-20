#include "propertywidgets/optionspropertywidget/optionspropertywidget.h"
#include "properties/typedproperty.h"
#include "propertywidgets/optionspropertywidget/optionsedit.h"

namespace omm
{

OptionsPropertyWidget::OptionsPropertyWidget(Scene& scene, const std::set<Property*>& properties)
  : PropertyWidget(scene, properties)
{
  auto options_edit = std::make_unique<OptionsEdit>([this](const std::size_t& value) {
    LOG(INFO) << "Set " << value;
    set_properties_value(value);
  });
  m_options_edit = options_edit.get();
  const auto get_options = std::mem_fn(&OptionsProperty::options);

  QSignalBlocker blocker(m_options_edit);
  m_options_edit->set_options(
    Property::get_value<std::vector<std::string>, OptionsProperty>(properties, get_options));
  set_default_layout(std::move(options_edit));
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
