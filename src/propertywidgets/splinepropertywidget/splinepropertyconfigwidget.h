#pragma once

#include "properties/splineproperty.h"
#include "propertywidgets/propertyconfigwidget.h"

namespace omm
{
class SplinePropertyConfigWidget : public PropertyConfigWidget<SplineProperty>
{
public:
  SplinePropertyConfigWidget();
  void init(const PropertyConfiguration&) override;
  void update(PropertyConfiguration&) const override;
};

}  // namespace omm
