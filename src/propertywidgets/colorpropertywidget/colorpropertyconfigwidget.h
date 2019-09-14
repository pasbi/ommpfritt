#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "properties/colorproperty.h"

namespace omm
{

class ColorPropertyConfigWidget : public PropertyConfigWidget
{
public:
  using PropertyConfigWidget::PropertyConfigWidget;
  static constexpr auto TYPE = "ColorPropertyConfigWidget";
  std::string type() const override { return TYPE; }
  void init(const Property::Configuration&) override { }
  void update(Property::Configuration&) const override { }
};

}  // namespace omm
