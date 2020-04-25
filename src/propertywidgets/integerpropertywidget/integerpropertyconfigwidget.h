#pragma once

#include "propertywidgets/numericpropertywidget/numericpropertyconfigwidget.h"
#include "properties/integerproperty.h"

namespace omm
{

class IntegerPropertyConfigWidget : public NumericPropertyConfigWidget<IntegerProperty>
{
public:
  using NumericPropertyConfigWidget<IntegerProperty>::NumericPropertyConfigWidget;
};

}  // namespace omm
