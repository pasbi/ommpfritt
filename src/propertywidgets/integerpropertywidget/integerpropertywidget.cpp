#include "propertywidgets/integerpropertywidget/integerpropertywidget.h"
#include "properties/property.h"

namespace omm
{
IntegerPropertyWidget::IntegerPropertyWidget(Scene& scene, const std::set<Property*>& properties)
    : NumericPropertyWidget<IntegerProperty>(scene, properties)
{
  const auto get_special_value = [](const IntegerProperty& ip) { return ip.special_value_label; };
  const auto special_value_label
      = Property::get_value<QString, IntegerProperty>(properties, get_special_value);
  if (!special_value_label.isEmpty()) {
    spinbox()->set_special_value_text(special_value_label);
  }
}

}  // namespace omm
