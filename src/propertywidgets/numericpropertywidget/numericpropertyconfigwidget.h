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
    auto [ min_edit, max_edit ] = NumericEdit<value_type>::make_range_edits();
    const auto update_range = [&numeric_property, &min_edit=min_edit, &max_edit=max_edit]() {
      numeric_property.set_range(min_edit->value(), max_edit->value());
    };
    QObject::connect(min_edit.get(), SIGNAL(value_changed()), this, SLOT(update_range()));
    QObject::connect(max_edit.get(), SIGNAL(value_changed()), this, SLOT(update_range()));
    const auto on_step_changed = [&numeric_property](value_type step) {
    };
    auto step_edit = std::make_unique<NumericEdit<value_type>>();
    QObject::connect(step_edit.get(), &AbstractNumericEdit::value_changed,
                     [&numeric_property, step_edit=step_edit.get()]() {
      numeric_property.set_step(step_edit->value());
    });
    step_edit->set_lower(NumericEditDetail::smallest_step<value_type>);
    step_edit->set_value(numeric_property.step());
    min_edit->set_value(numeric_property.lower());
    max_edit->set_value(numeric_property.upper());

    this->form_layout()->addRow(QObject::tr("min", "NumericProperty"), min_edit.release());
    this->form_layout()->addRow(QObject::tr("max", "NumericProperty"), max_edit.release());
    this->form_layout()->addRow(QObject::tr("step", "NumericProperty"), step_edit.release());
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
