#pragma once

#include "propertywidgets/numericpropertywidget/numericpropertyconfigwidget.h"

namespace omm
{

class FloatProperty;

class FloatPropertyConfigWidget : public NumericPropertyConfigWidget<FloatProperty>
{
public:
  using NumericPropertyConfigWidget<FloatProperty>::NumericPropertyConfigWidget;
};

}  // namespace omm
