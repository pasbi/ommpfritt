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
  QtSpinBox(const std::function<void(double)>& on_value_changed)
  {
    const auto s = static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged);
    connect(this, s, on_value_changed);
  }
  using QDoubleSpinBox::QDoubleSpinBox;
  using Base = QDoubleSpinBox;
};

template<> class QtSpinBox<int> : public QSpinBox
{
public:
  QtSpinBox(const std::function<void(int)>& on_value_changed)
  {
    connect(this, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), on_value_changed);
  }
  using QSpinBox::QSpinBox;
  using Base = QSpinBox;
};

template<typename T> class SpinBox : public QtSpinBox<T>, public MultiValueEdit<T>
{
public:
  using value_type = T;
  explicit SpinBox(const typename MultiValueEdit<T>::on_value_changed_t& on_value_changed)
    : QtSpinBox<T>(on_value_changed)
    , MultiValueEdit<T>(on_value_changed)
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
