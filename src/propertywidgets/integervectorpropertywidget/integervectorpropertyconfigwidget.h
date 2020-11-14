#pragma once

#include "properties/integervectorproperty.h"
#include "propertywidgets/vectorpropertywidget/vectorpropertyconfigwidget.h"

namespace omm
{
class IntegerVectorPropertyConfigWidget : public VectorPropertyConfigWidget<IntegerVectorProperty>
{
  Q_OBJECT
public:
  using VectorPropertyConfigWidget<IntegerVectorProperty>::VectorPropertyConfigWidget;
};

}  // namespace omm
