#pragma once

#include "propertywidgets/propertywidget.h"
#include "propertywidgets/multivalueedit.h"
#include "propertywidgets/numericpropertywidget/spinbox.h"
#include "properties/integerproperty.h"
#include "properties/floatproperty.h"

namespace omm
{

template<typename NumericPropertyT>
class NumericPropertyWidget : public PropertyWidget<NumericPropertyT>
{
public:
  using value_type = typename NumericPropertyT::value_type;
  explicit NumericPropertyWidget(Scene& scene, const std::set<Property*>& properties);

protected:
  void update_edit() override;

private:
  SpinBox<value_type>* m_spinbox;
};

class IntegerPropertyWidget : public NumericPropertyWidget<IntegerProperty>
{
public:
  using NumericPropertyWidget::NumericPropertyWidget;
  std::string type() const override { return TYPE; }
  static constexpr auto TYPE = "IntegerPropertyWidget";
};

class FloatPropertyWidget : public NumericPropertyWidget<FloatProperty>
{
public:
  using NumericPropertyWidget::NumericPropertyWidget;
  std::string type() const override { return TYPE; }
  static constexpr auto TYPE = "FloatPropertyWidget";
};

}  // namespace omm
