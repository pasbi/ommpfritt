#pragma once

#include "propertywidgets/numericpropertywidget/numericpropertyconfigwidget.h"

namespace omm
{

class IntegerProperty;

class IntegerPropertyConfigWidget : public NumericPropertyConfigWidget<IntegerProperty>
{
  Q_OBJECT
};

}  // namespace omm
