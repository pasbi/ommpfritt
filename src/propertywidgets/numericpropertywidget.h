#pragma once

#include "propertywidgets/propertywidget.h"
#include "propertywidgets/multivalueedit.h"
#include "properties/integerproperty.h"
#include "properties/floatproperty.h"

namespace omm
{

template<typename NumericPropertyT>
class NumericPropertyWidget : public PropertyWidget<NumericPropertyT>
{
public:
  using value_type = typename NumericPropertyT::value_type;
  explicit NumericPropertyWidget(Scene& scene, const Property::SetOfProperties& properties);

protected:
  void on_property_value_changed() override;

private:
  class SpinBox;
  SpinBox* m_spinbox;
};

class IntegerPropertyWidget : public NumericPropertyWidget<IntegerProperty>
{
public:
  using NumericPropertyWidget::NumericPropertyWidget;
  std::string type() const override { return "IntegerPropertyWidget"; }
};

class FloatPropertyWidget : public NumericPropertyWidget<FloatProperty>
{
public:
  using NumericPropertyWidget::NumericPropertyWidget;
  std::string type() const override { return "FloatPropertyWidget"; }
};

}  // namespace omm
