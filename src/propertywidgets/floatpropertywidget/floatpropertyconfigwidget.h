#pragma once

#include "propertywidgets/numericpropertywidget/numericpropertyconfigwidget.h"

namespace omm
{

class FloatProperty;

class FloatPropertyConfigWidget : public NumericPropertyConfigWidget<FloatProperty>
{
  Q_OBJECT
};

}  // namespace omm
