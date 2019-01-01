#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "properties/transformationproperty.h"

namespace omm
{

class TransformationPropertyConfigWidget : public PropertyConfigWidget<TransformationProperty>
{
public:
  using PropertyConfigWidget::PropertyConfigWidget;
  std::string type() const override;
};

}  // namespace omm
