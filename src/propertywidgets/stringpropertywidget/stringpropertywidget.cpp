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
  const auto get_line_mode = std::mem_fn(&StringProperty::line_mode);
  const auto line_mode
    = Property::get_value<StringProperty::LineMode, StringProperty>(properties, get_line_mode);

  const auto make_edit = [this](auto line_mode) -> std::unique_ptr<AbstractGenericTextEdit> {
    switch (line_mode)
    {
    case StringProperty::LineMode::SingleLine:
      return std::make_unique<SingleLineTextEdit>(this);
    case StringProperty::LineMode::MultiLine:
      return std::make_unique<MultiLineTextEdit>(this);
    }
  };

  auto text_edit = make_edit(line_mode);
  m_text_edit = text_edit.get();
  set_default_layout(std::move(text_edit));

  connect(m_text_edit, &AbstractGenericTextEdit::text_changed, [this]() {
    set_properties_value(m_text_edit->value());
  });

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
