#pragma once

#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLineEdit>
#include "propertywidgets/multivalueedit.h"
#include "widgets/numericedit.h"

namespace omm
{

template<typename T> class NumericMultiValueEdit : public NumericEdit<T>, public MultiValueEdit<T>
{
public:
  using value_type = T;
  using on_value_changed_t = std::function<void(T)>;
  NumericMultiValueEdit(const on_value_changed_t& on_value_changed)
    : NumericEdit<T>(on_value_changed), MultiValueEdit<T>(on_value_changed)
  {}
  void set_value(const T& value) override { NumericEdit<T>::set_value(value); }
  T value() const override { return NumericEdit<T>::value(); }
  void set_inconsistent_value() override { this->set_invalid_value(); }
};

}  // namespace omm
