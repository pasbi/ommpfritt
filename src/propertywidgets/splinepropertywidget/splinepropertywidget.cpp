#include "propertywidgets/splinepropertywidget/splinepropertywidget.h"

namespace omm
{

SplinePropertyWidget::SplinePropertyWidget(Scene& scene, const std::set<Property*>& properties)
  : PropertyWidget<SplineProperty>(scene, properties)
{

}

void SplinePropertyWidget::update_edit()
{

}

}  // namespace omm
