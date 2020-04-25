#pragma once

#include "propertywidgets/numericpropertywidget/numericpropertywidget.h"
#include "properties/floatproperty.h"

namespace omm
{

class FloatPropertyWidget : public NumericPropertyWidget<FloatProperty>
{
public:
  using NumericPropertyWidget::NumericPropertyWidget;
  static constexpr auto TYPE = "FloatPropertyWidget";
};

}  // namespace omm
