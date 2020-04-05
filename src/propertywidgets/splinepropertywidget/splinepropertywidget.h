#pragma once

#include <string>
#include "propertywidgets/propertywidget.h"
#include "properties/splineproperty.h"

namespace omm
{

class SplineWidget;

class SplinePropertyWidget : public PropertyWidget<SplineProperty>
{
public:
  explicit SplinePropertyWidget(Scene& scene, const std::set<Property*>& properties);

protected:
  void update_edit() override;
  QString type() const override;

private:
  SplineWidget* m_spline_edit;
};

}  // namespace omm
