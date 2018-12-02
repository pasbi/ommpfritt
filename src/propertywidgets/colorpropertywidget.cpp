#include "propertywidgets/colorpropertywidget.h"
#include "properties/typedproperty.h"
#include "propertywidgets/coloredit.h"

namespace omm
{

ColorPropertyWidget::ColorPropertyWidget(Scene& scene, const Property::SetOfProperties& properties)
  : PropertyWidget(scene, properties)
{
  auto color_edit = std::make_unique<ColorEdit>();
  m_color_edit = color_edit.get();
  set_default_layout(std::move(color_edit));

  connect( m_color_edit, &ColorEdit::color_changed,
           this, &ColorPropertyWidget::set_properties_value );

  on_property_value_changed();
}

void ColorPropertyWidget::on_property_value_changed()
{
  m_color_edit->blockSignals(true);
  m_color_edit->set_values(get_properties_values());
  m_color_edit->blockSignals(false);
}

std::string ColorPropertyWidget::type() const
{
  return "ColorPropertyWidget";
}

}  // namespace omm
