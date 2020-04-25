#pragma once

#include "propertywidgets/numericpropertywidget/numericpropertyconfigwidget.h"
#include "properties/floatproperty.h"

namespace omm
{

class FloatPropertyConfigWidget : public NumericPropertyConfigWidget<FloatProperty>
{
public:
  using NumericPropertyConfigWidget<FloatProperty>::NumericPropertyConfigWidget;
};

}  // namespace omm
