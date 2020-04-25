#pragma once

#include "propertywidgets/numericpropertywidget/numericpropertywidget.h"
#include "properties/integerproperty.h"

namespace omm
{

class IntegerPropertyWidget : public NumericPropertyWidget<IntegerProperty>
{
public:
  using NumericPropertyWidget::NumericPropertyWidget;
  static constexpr auto TYPE = "IntegerPropertyWidget";
};

}  // namespace omm
