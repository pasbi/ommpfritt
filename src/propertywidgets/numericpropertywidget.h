#pragma once

#include "propertywidgets/propertywidget.h"
#include "propertywidgets/multivalueedit.h"

namespace omm
{

template<typename T>
class NumericPropertyWidget : public PropertyWidget<T>
{
public:
  using SetOfProperties = typename PropertyWidget<T>::SetOfProperties;
  explicit NumericPropertyWidget(const SetOfProperties& properties);

protected:
  void on_value_changed() override;

private:
  class SpinBox;
  SpinBox* m_spinbox;
};

class IntegerPropertyWidget : public NumericPropertyWidget<int>
{
public:
  using NumericPropertyWidget::NumericPropertyWidget;
  std::string type() const override { return "IntegerPropertyWidget"; }
};

class FloatPropertyWidget : public NumericPropertyWidget<double>
{
public:
  using NumericPropertyWidget::NumericPropertyWidget;
  std::string type() const override { return "FloatPropertyWidget"; }
};

}  // namespace omm
