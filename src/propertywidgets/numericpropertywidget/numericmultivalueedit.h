#pragma once

#include "propertywidgets/multivalueedit.h"
#include "widgets/numericedit.h"
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QSpinBox>

namespace omm
{
template<typename T>
class NumericMultiValueEdit
    : public NumericEdit<T>
    , public MultiValueEdit<T>
{
public:
  using value_type = T;
  using NumericEdit<T>::NumericEdit;
  void set_value(const T& value) override
  {
    NumericEdit<T>::set_value(value);
  }
  T value() const override
  {
    return NumericEdit<T>::value();
  }
  void set_inconsistent_value() override
  {
    this->set_invalid_value();
  }
};

}  // namespace omm
