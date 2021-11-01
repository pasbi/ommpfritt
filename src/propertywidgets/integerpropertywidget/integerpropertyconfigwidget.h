#pragma once

#include "propertywidgets/numericpropertywidget/numericpropertyconfigwidget.h"

namespace omm
{

class IntegerProperty;

class IntegerPropertyConfigWidget : public NumericPropertyConfigWidget<IntegerProperty>
{
public:
  using NumericPropertyConfigWidget<IntegerProperty>::NumericPropertyConfigWidget;
};

}  // namespace omm
