#include "propertywidgets/splinepropertywidget/splinepropertywidget.h"
#include "propertywidgets/splinepropertywidget/splinewidget.h"

namespace omm
{
SplinePropertyWidget::SplinePropertyWidget(Scene& scene, const std::set<Property*>& properties)
    : PropertyWidget<SplineProperty>(scene, properties)
{
  auto spline_edit = std::make_unique<SplineWidget>();
  m_spline_edit = spline_edit.get();
  connect(spline_edit.get(), &SplineWidget::value_changed, [this](const SplineType& spline) {
    set_properties_value(spline);
  });
  set_widget(std::move(spline_edit));
  SplinePropertyWidget::update_edit();
}

void SplinePropertyWidget::update_edit()
{
  QSignalBlocker blocker(m_spline_edit);
  m_spline_edit->set_values(get_properties_values());
}

}  // namespace omm
