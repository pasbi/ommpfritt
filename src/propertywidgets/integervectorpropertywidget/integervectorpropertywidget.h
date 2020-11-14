#pragma once

#include "properties/integervectorproperty.h"
#include "propertywidgets/vectorpropertywidget/vectorpropertywidget.h"

namespace omm
{
class IntegerVectorPropertyWidget : public VectorPropertyWidget<IntegerVectorProperty>
{
public:
  using VectorPropertyWidget::VectorPropertyWidget;
};

}  // namespace omm
