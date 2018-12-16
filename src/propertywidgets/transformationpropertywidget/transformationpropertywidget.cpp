#include "propertywidgets/transformationpropertywidget/transformationpropertywidget.h"
#include "propertywidgets/transformationpropertywidget/spinboxtransformationedit.h"
#include "propertywidgets/multivalueedit.h"

namespace omm
{

TransformationPropertyWidget
::TransformationPropertyWidget(Scene& scene, const Property::SetOfProperties& properties)
  : PropertyWidget(scene, properties)
{
  auto transformation_edit = std::make_unique<SpinBoxTransformationEdit>();
  m_transformation_edit = transformation_edit.get();
  set_default_layout(std::move(transformation_edit));

  QObject::connect( m_transformation_edit, &AbstractTransformationEdit::value_changed,
                    [this](const ObjectTransformation& value) { set_properties_value(value); } );

  update_edit();
}

void TransformationPropertyWidget::update_edit()
{
  QSignalBlocker blocker(m_transformation_edit);
  m_transformation_edit->set_values(get_properties_values());
}

std::string TransformationPropertyWidget::type() const
{
  return "TransformationPropertyWidget";
}


} // namespace omm