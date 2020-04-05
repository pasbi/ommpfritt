#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "properties/splineproperty.h"

namespace omm
{

class SplinePropertyConfigWidget : public PropertyConfigWidget<SplineProperty>
{
public:
  SplinePropertyConfigWidget();
  void init(const Property::Configuration &configuration) override;
  void update(Property::Configuration &configuration) const override;
};

}  // namespace omm
