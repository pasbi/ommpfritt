#include "propertywidgets/stringpropertywidget/stringpropertywidget.h"

#include "propertywidgets/multivalueedit.h"
#include "properties/typedproperty.h"
#include "propertywidgets/stringpropertywidget/generictextedit.h"

namespace omm
{

StringPropertyWidget
::StringPropertyWidget(Scene& scene, const std::set<Property*>& properties)
  : PropertyWidget(scene, properties)
{
  const auto get_is_multi_line = std::mem_fn(&StringProperty::is_multi_line);
  const auto is_multi_line
    = Property::get_value<bool, StringProperty>(properties, get_is_multi_line);

  const auto make_edit = [this](auto is_multi_line) -> std::unique_ptr<AbstractGenericTextEdit> {
    if (is_multi_line) {
      return std::make_unique<MultiLineTextEdit>(this, [this](const auto& value) {
        set_properties_value(value);
      });
    } else {
      return std::make_unique<SingleLineTextEdit>(this, [this](const auto& value) {
        set_properties_value(value);
      });
    }
  };

  auto text_edit = make_edit(is_multi_line);
  m_text_edit = text_edit.get();
  set_default_layout(std::move(text_edit));

  update_edit();
}

void StringPropertyWidget::update_edit()
{
  QSignalBlocker blocker(m_text_edit);
  m_text_edit->set_values(get_properties_values());
}

std::string StringPropertyWidget::type() const
{
  return TYPE;
}

}  // namespace omm
