#pragma once

#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLineEdit>
#include "propertywidgets/multivalueedit.h"

namespace omm
{

template<typename T> class QtSpinBox;

template<> class QtSpinBox<double> : public QDoubleSpinBox
{
public:
  using QDoubleSpinBox::QDoubleSpinBox;
  using Base = QDoubleSpinBox;
};

template<> class QtSpinBox<int> : public QSpinBox
{
public:
  using QSpinBox::QSpinBox;
  using Base = QSpinBox;
};

template<typename T> class SpinBox : public QtSpinBox<T>, public MultiValueEdit<T>
{
public:
  using value_type = T;
  explicit SpinBox()
  {
    QtSpinBox<T>::setRange(-1000, 1000);
  }

  void set_value(const value_type& value) override
  {
    QtSpinBox<T>::setValue(value);
  }

  value_type value() const override
  {
    return QtSpinBox<T>::value();
  }

  void set_inconsistent_value() override
  {
    QtSpinBox<T>::lineEdit()->setText("");
  }
};

}  // namespace omm
