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
    auto min_widget = std::make_unique<NumericEdit<value_type>>([](const value_type&) {});
    this->form_layout()->addRow(NumericPropertyConfigWidget::tr("min"), min_widget.release());
    auto max_widget = std::make_unique<NumericEdit<value_type>>([](const value_type&) {});
    this->form_layout()->addRow(NumericPropertyConfigWidget::tr("max"), max_widget.release());
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
