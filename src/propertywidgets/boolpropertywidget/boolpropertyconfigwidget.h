#pragma once

#include "propertywidgets/propertyconfigwidget.h"

namespace omm
{
class BoolProperty;
class BoolPropertyConfigWidget : public PropertyConfigWidget<BoolProperty>
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
