#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "properties/triggerproperty.h"

namespace omm
{

class TriggerPropertyConfigWidget : public PropertyConfigWidget<TriggerProperty>
{
public:
  using PropertyConfigWidget::PropertyConfigWidget;
  std::string type() const override;
};

}  // namespace omm
