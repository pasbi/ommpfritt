#include "propertywidgets/colorpropertywidget/colorpropertywidget.h"
#include "properties/typedproperty.h"
#include "propertywidgets/colorpropertywidget/coloredit.h"

namespace omm
{
ColorPropertyWidget::ColorPropertyWidget(Scene& scene, const std::set<Property*>& properties)
    : PropertyWidget(scene, properties)
{
  auto color_edit = std::make_unique<ColorEdit>();
  connect(color_edit.get(), &ColorEdit::value_changed, [this](const Color& color) {
    set_properties_value(color);
  });
  m_color_edit = color_edit.get();
  color_edit->text = label();
  set_widget(std::move(color_edit));

  ColorPropertyWidget::update_edit();
}

void ColorPropertyWidget::update_edit()
{
  QSignalBlocker blocker(m_color_edit);
  m_color_edit->set_values(get_properties_values());
}

}  // namespace omm
