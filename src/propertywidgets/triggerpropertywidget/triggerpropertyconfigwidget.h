#pragma once

#include "propertywidgets/propertyconfigwidget.h"

namespace omm
{

class TriggerProperty;

class TriggerPropertyConfigWidget : public PropertyConfigWidget<TriggerProperty>
{
public:
  using PropertyConfigWidget::PropertyConfigWidget;
  void init(const PropertyConfiguration&) override
  {
  }
  void update(PropertyConfiguration&) const override
  {
  }
};

}  // namespace omm
