#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "properties/triggerproperty.h"

namespace omm
{

class TriggerPropertyConfigWidget : public PropertyConfigWidget
{
public:
  using PropertyConfigWidget::PropertyConfigWidget;
  static constexpr auto TYPE = "TriggerPropertyConfigWidget";
  std::string type() const override { return TYPE; }
  void init(const Property::Configuration&) override { }
  void update(Property::Configuration&) const override { }
};

}  // namespace omm
