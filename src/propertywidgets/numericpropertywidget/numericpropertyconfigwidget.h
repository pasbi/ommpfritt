#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "properties/integerproperty.h"
#include "properties/floatproperty.h"

#include <QHBoxLayout>
#include <QFormLayout>
#include "widgets/numericedit.h"

namespace omm
{

template<typename PropertyT>
class NumericPropertyConfigWidget : public PropertyConfigWidget<PropertyT>
{
public:
  using value_type = typename PropertyT::value_type;
  NumericPropertyConfigWidget(QWidget* parent, Property& property)
    : PropertyConfigWidget<PropertyT>(parent, property)
  {
    auto& numeric_property = type_cast<PropertyT&>(property);
    const auto on_min_max_changed = [&numeric_property](const value_type min, const value_type max)
    {
      numeric_property.set_range(min, max);
    };
    auto [ min_edit, max_edit ] = NumericEdit<value_type>::make_min_max_edits(on_min_max_changed);
    const auto on_step_changed = [&numeric_property](value_type step) {
      if (step == 0) {

      }
      numeric_property.set_step(step);
    };
    auto step_edit = std::make_unique<NumericEdit<value_type>>(on_step_changed);
    if constexpr (std::is_floating_point_v<value_type>) {
      step_edit->set_lower(std::numeric_limits<value_type>::min());
    } else {
      step_edit->set_lower(1);
    }
    step_edit->set_value(numeric_property.step());
    min_edit->set_value(numeric_property.lower());
    max_edit->set_value(numeric_property.upper());

    this->form_layout()->addRow(NumericPropertyConfigWidget::tr("min"), min_edit.release());
    this->form_layout()->addRow(NumericPropertyConfigWidget::tr("max"), max_edit.release());
    this->form_layout()->addRow(NumericPropertyConfigWidget::tr("step"), step_edit.release());
  }
};

class IntegerPropertyConfigWidget : public NumericPropertyConfigWidget<IntegerProperty>
{
public:
  using NumericPropertyConfigWidget::NumericPropertyConfigWidget;
  std::string type() const override;
};

class FloatPropertyConfigWidget : public NumericPropertyConfigWidget<FloatProperty>
{
public:
  using NumericPropertyConfigWidget::NumericPropertyConfigWidget;
  std::string type() const override;
};

}  // namespace omm
