#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "properties/splineproperty.h"

namespace omm
{

class SplinePropertyConfigWidget : public PropertyConfigWidget
{
public:
  SplinePropertyConfigWidget();
  static constexpr auto TYPE = "SplinePropertyConfigWidget";
  QString type() const override { return TYPE; }
  void init(const Property::Configuration &configuration) override;
  void update(Property::Configuration &configuration) const override;
};

}  // namespace omm
