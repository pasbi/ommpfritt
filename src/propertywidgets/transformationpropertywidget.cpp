#include "propertywidgets/transformationpropertywidget.h"
#include "propertywidgets/spinboxtransformationedit.h"
#include "propertywidgets/multivalueedit.h"

namespace omm
{

TransformationPropertyWidget
::TransformationPropertyWidget(Scene& scene, const Property::SetOfProperties& properties)
  : PropertyWidget<ObjectTransformation>(scene, properties)
{
  auto transformation_edit = std::make_unique<SpinBoxTransformationEdit>();
  m_transformation_edit = transformation_edit.get();
  set_default_layout(std::move(transformation_edit));

  QObject::connect( m_transformation_edit, &AbstractTransformationEdit::value_changed,
                    [this](const auto& value) { set_properties_value(value); } );

  on_value_changed();
}

void TransformationPropertyWidget::on_value_changed()
{
  m_transformation_edit->blockSignals(true);
  m_transformation_edit->set_values(get_properties_values());
  m_transformation_edit->blockSignals(false);
}

std::string TransformationPropertyWidget::type() const
{
  return "TransformationPropertyWidget";
}


} // namespace omm