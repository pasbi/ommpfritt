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
  auto line_edit = std::make_unique<LineEdit>();
  m_line_edit = line_edit.get();
  set_default_layout(std::move(line_edit));

  connect(m_line_edit, &QLineEdit::textChanged, [this](const QString& text) {
    set_properties_value(text.toStdString());
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
