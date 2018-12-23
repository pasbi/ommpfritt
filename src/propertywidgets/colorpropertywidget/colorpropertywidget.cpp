#include "propertywidgets/colorpropertywidget/colorpropertywidget.h"
#include "properties/typedproperty.h"
#include "propertywidgets/colorpropertywidget/coloredit.h"

namespace omm
{

ColorPropertyWidget::ColorPropertyWidget(Scene& scene, const std::set<Property*>& properties)
  : PropertyWidget(scene, properties)
{
  auto color_edit = std::make_unique<ColorEdit>();
  m_color_edit = color_edit.get();
  set_default_layout(std::move(color_edit));

  connect( m_color_edit, &ColorEdit::color_changed,
           this, &ColorPropertyWidget::set_properties_value );

  update_edit();
}

void ColorPropertyWidget::update_edit()
{
  QSignalBlocker blocker(m_color_edit);
  m_color_edit->set_values(get_properties_values());
}

std::string ColorPropertyWidget::type() const
{
  return "ColorPropertyWidget";
}

}  // namespace omm
