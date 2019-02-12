#include "propertywidgets/colorpropertywidget/colorpropertywidget.h"
#include "properties/typedproperty.h"
#include "propertywidgets/colorpropertywidget/coloredit.h"

namespace omm
{

ColorPropertyWidget::ColorPropertyWidget( Scene& scene, const std::string& key,
                                          const std::set<Property*>& properties )
  : PropertyWidget(scene, key, properties)
{
  auto color_edit = std::make_unique<ColorEdit>([this](const Color& value) {
    set_properties_value(value);
  });
  m_color_edit = color_edit.get();
  set_default_layout(std::move(color_edit));

  update_edit();
}

void ColorPropertyWidget::update_edit()
{
  QSignalBlocker blocker(m_color_edit);
  m_color_edit->set_values(get_properties_values());
}

std::string ColorPropertyWidget::type() const
{
  return TYPE;
}

}  // namespace omm
