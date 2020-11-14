#pragma once

#include "properties/floatvectorproperty.h"
#include "propertywidgets/vectorpropertywidget/vectorpropertywidget.h"

namespace omm
{
class FloatVectorPropertyWidget : public VectorPropertyWidget<FloatVectorProperty>
{
public:
  using VectorPropertyWidget::VectorPropertyWidget;
};

}  // namespace omm
