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
  const auto getter = std::mem_fn(&StringProperty::mode);
  const auto mode = Property::get_value<StringProperty::Mode, StringProperty>(properties, getter);

  std::unique_ptr<AbstractGenericTextEdit> edit;
  const auto spv_closure = [this](const auto& value) { set_properties_value(value); };
  switch (mode) {
  case StringProperty::Mode::MultiLine:
    edit = std::make_unique<MultiLineTextEdit>(this, spv_closure);
    break;
  case StringProperty::Mode::SingleLine:
    edit = std::make_unique<SingleLineTextEdit>(this, spv_closure);
    break;
  case StringProperty::Mode::FilePath:
    edit = std::make_unique<FilePathTextEdit>(this, spv_closure);
    break;
  }

  m_text_edit = edit.get();
  set_default_layout(std::move(edit));

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
