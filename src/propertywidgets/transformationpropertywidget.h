#pragma once

#include "propertywidgets/propertywidget.h"

namespace omm
{

class AbstractTransformationEdit;

class TransformationPropertyWidget : public PropertyWidget<ObjectTransformation>
{
public:
  explicit TransformationPropertyWidget(Scene& scene, const Property::SetOfProperties& properties);
  std::string type() const override;

protected:
  void on_value_changed() override;

  AbstractTransformationEdit* m_transformation_edit;
};

}  // namespace omm
