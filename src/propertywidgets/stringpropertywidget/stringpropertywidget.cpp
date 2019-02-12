#include "propertywidgets/stringpropertywidget/stringpropertywidget.h"

#include "propertywidgets/multivalueedit.h"
#include "properties/typedproperty.h"
#include "propertywidgets/stringpropertywidget/texteditadapter.h"

namespace omm
{

StringPropertyWidget
::StringPropertyWidget(Scene& scene, const std::string& key, const std::set<Property*>& properties)
  : PropertyWidget(scene, key, properties)
{
  const auto getter = std::mem_fn(&StringProperty::mode);
  const auto mode = Property::get_value<StringProperty::Mode, StringProperty>(properties, getter);

  std::unique_ptr<AbstractTextEditAdapter> edit;
  const auto spv_closure = [this](const auto& value) { set_properties_value(value); };
  switch (mode) {
  case StringProperty::Mode::MultiLine:
    edit = std::make_unique<TextEditAdapter<QTextEdit>>(spv_closure, this);
    break;
  case StringProperty::Mode::SingleLine:
    edit = std::make_unique<TextEditAdapter<QLineEdit>>(spv_closure, this);
    break;
  case StringProperty::Mode::FilePath:
    edit = std::make_unique<TextEditAdapter<FilePathEdit>>(spv_closure, this);
    break;
  case StringProperty::Mode::Code:
    edit = std::make_unique<TextEditAdapter<CodeEdit>>(spv_closure, this);
  }

  m_text_edit = edit.get();
  auto text_edit_widget = std::unique_ptr<QWidget>(edit.release()->as_widget());
  set_default_layout(std::move(text_edit_widget));

  update_edit();
}

void StringPropertyWidget::update_edit()
{
  QSignalBlocker blocker(m_text_edit->as_widget());
  m_text_edit->set_values(get_properties_values());
}

std::string StringPropertyWidget::type() const
{
  return TYPE;
}

}  // namespace omm
