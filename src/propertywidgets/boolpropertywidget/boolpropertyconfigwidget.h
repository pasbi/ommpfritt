#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "properties/boolproperty.h"

namespace omm
{

class BoolPropertyConfigWidget : public PropertyConfigWidget
{
public:
  using PropertyConfigWidget::PropertyConfigWidget;
  static constexpr auto TYPE = "BoolPropertyConfigWidget";
  std::string type() const override { return TYPE; }
  void init(const Property::Configuration&) override { }
  void update(Property::Configuration&) const override { }
};

}  // namespace omm
