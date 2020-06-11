#pragma once

#include "propertywidgets/numericpropertywidget/numericpropertywidget.h"
#include "properties/integerproperty.h"

namespace omm
{

class IntegerPropertyWidget : public NumericPropertyWidget<IntegerProperty>
{
public:
  IntegerPropertyWidget(Scene& scene, const std::set<Property*>& properties);
  static constexpr auto TYPE = "IntegerPropertyWidget";
};

}  // namespace omm
