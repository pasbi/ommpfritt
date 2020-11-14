#pragma once

#include "properties/integerproperty.h"
#include "propertywidgets/numericpropertywidget/numericpropertyconfigwidget.h"

namespace omm
{
class IntegerPropertyConfigWidget : public NumericPropertyConfigWidget<IntegerProperty>
{
public:
  using NumericPropertyConfigWidget<IntegerProperty>::NumericPropertyConfigWidget;
};

}  // namespace omm
