#include "propertywidgets/stringpropertywidget/stringpropertywidget.h"

#include "propertywidgets/multivalueedit.h"
#include "properties/typedproperty.h"
#include "propertywidgets/stringpropertywidget/lineedit.h"

namespace omm
{

StringPropertyWidget
::StringPropertyWidget(Scene& scene, const Property::SetOfProperties& properties)
  : PropertyWidget(scene, properties)
{
  const auto get_line_mode = std::mem_fn(&StringProperty::line_mode);
  auto line_mode = Property::get_value<StringProperty::LineMode, StringProperty>( properties,
                                                                                  get_line_mode );
  auto line_edit = std::make_unique<LineEdit>(line_mode);
  m_line_edit = line_edit.get();
  set_default_layout(std::move(line_edit));

  connect(m_line_edit, &LineEdit::textChanged, [this]() {
    set_properties_value(m_line_edit->value());
  });

  update_edit();
}

void StringPropertyWidget::update_edit()
{
  QSignalBlocker blocker(m_line_edit);
  m_line_edit->set_values(get_properties_values());
}

std::string StringPropertyWidget::type() const
{
  return "StringPropertyWidget";
}

}  // namespace omm
