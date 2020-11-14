#pragma once

#include "properties/splineproperty.h"
#include "propertywidgets/propertyconfigwidget.h"

namespace omm
{
class SplinePropertyConfigWidget : public PropertyConfigWidget<SplineProperty>
{
public:
  SplinePropertyConfigWidget();
  void init(const Property::Configuration&) override;
  void update(Property::Configuration&) const override;
};

}  // namespace omm
