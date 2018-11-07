#pragma once

#include "propertywidgets/propertywidget.h"

namespace omm
{

class TransformationPropertyWidget : public PropertyWidget<ObjectTransformation>
{
public:
  using PropertyWidget<ObjectTransformation>::PropertyWidget;
protected:
  void on_value_changed() override {}
  std::string type() const override;
};

}  // namespace omm
