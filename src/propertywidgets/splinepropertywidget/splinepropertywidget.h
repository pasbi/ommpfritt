#pragma once

#include "properties/splineproperty.h"
#include "propertywidgets/propertywidget.h"
#include <string>

namespace omm
{
class SplineWidget;

class SplinePropertyWidget : public PropertyWidget<SplineProperty>
{
public:
  explicit SplinePropertyWidget(Scene& scene, const std::set<Property*>& properties);

protected:
  void update_edit() override;

private:
  SplineWidget* m_spline_edit;
};

}  // namespace omm
