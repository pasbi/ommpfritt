#pragma once

#include "properties/floatvectorproperty.h"
#include "propertywidgets/vectorpropertywidget/vectorpropertyconfigwidget.h"

namespace omm
{
class FloatVectorPropertyConfigWidget : public VectorPropertyConfigWidget<FloatVectorProperty>
{
  Q_OBJECT
public:
  using VectorPropertyConfigWidget<FloatVectorProperty>::VectorPropertyConfigWidget;
};

}  // namespace omm
