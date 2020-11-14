#pragma once

#include "properties/floatproperty.h"
#include "propertywidgets/numericpropertywidget/numericpropertywidget.h"

namespace omm
{
class FloatPropertyWidget : public NumericPropertyWidget<FloatProperty>
{
public:
  using NumericPropertyWidget::NumericPropertyWidget;
  static constexpr auto TYPE = "FloatPropertyWidget";
};

}  // namespace omm
