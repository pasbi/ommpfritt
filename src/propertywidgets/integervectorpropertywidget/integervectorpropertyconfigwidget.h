#pragma once

#include "properties/integervectorproperty.h"
#include "propertywidgets/vectorpropertywidget/vectorpropertyconfigwidget.h"

namespace omm
{
class IntegerVectorPropertyConfigWidget : public VectorPropertyConfigWidget<IntegerVectorProperty>
{
  Q_OBJECT
};

}  // namespace omm
