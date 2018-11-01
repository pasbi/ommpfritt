#pragma once

#include "objects/objecttransformation.h"
#include "propertywidgets/propertywidget.h"

namespace omm
{

class ObjectTransformationPropertyWidget : public PropertyWidget<ObjectTransformation>
{
public:
  using PropertyWidget<ObjectTransformation>::PropertyWidget;
protected:
  void on_value_changed() override {}
};

}  // namespace omm
