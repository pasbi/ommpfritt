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
    const auto on_range_changed = [&numeric_property](const value_type min, const value_type max)
    {
      numeric_property.set_range(min, max);
    };
    auto [ min_edit, max_edit ] = NumericEdit<value_type>::make_range_edits(on_range_changed);
    const auto on_step_changed = [&numeric_property](value_type step) {
      numeric_property.set_step(step);
    };
    auto step_edit = std::make_unique<NumericEdit<value_type>>(on_step_changed);
    step_edit->set_lower(NumericEditDetail::smallest_step<value_type>);
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
