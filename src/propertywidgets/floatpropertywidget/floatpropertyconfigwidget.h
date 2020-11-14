#pragma once

#include "properties/floatproperty.h"
#include "propertywidgets/numericpropertywidget/numericpropertyconfigwidget.h"

namespace omm
{
class FloatPropertyConfigWidget : public NumericPropertyConfigWidget<FloatProperty>
{
public:
  using NumericPropertyConfigWidget<FloatProperty>::NumericPropertyConfigWidget;
};

}  // namespace omm
