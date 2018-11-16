#include "propertywidgets/numericpropertywidget.h"

#include <QSpinBox>
#include <QLineEdit>

// TODO units
// TODO alternative editor widgets (line edit, slider)
// TODO min, max, step, default

namespace
{

template<typename T> class SpinBox;

template<> class SpinBox<double> : public QDoubleSpinBox
{
public:
  explicit SpinBox()
  {
    setRange(-1000, 1000);
  }
  using Base = QDoubleSpinBox;
};

template<> class SpinBox<int> : public QSpinBox
{
public:
  explicit SpinBox()
  {
    setRange(-1000, 1000);
  }
  using Base = QSpinBox;
};

}  // namespace


namespace omm
{

template<typename T>
class NumericPropertyWidget<T>::SpinBox : public ::SpinBox<T>, public MultiValueEdit<T>
{
public:
  explicit SpinBox() {}
  void set_value(const T& value) override { ::SpinBox<T>::setValue(value); }
  T value() const override { return SpinBox::value(); }

protected:
  void set_inconsistent_value() override {
    NumericPropertyWidget<T>::SpinBox::lineEdit()->setText("");
  }
};

template<typename T> NumericPropertyWidget<T>
::NumericPropertyWidget(Scene& scene, const Property::SetOfProperties& properties)
  : PropertyWidget<T>(scene, properties)
{
  auto spinbox = std::make_unique<SpinBox>();
  m_spinbox = spinbox.get();
  this->set_default_layout(std::move(spinbox));

  const auto set_properties_value = [this](T value) {
    this->set_properties_value(value);
  };
  QObject::connect( m_spinbox,
                    static_cast<void(SpinBox::Base::*)(T)>(&SpinBox::Base::valueChanged),
                    set_properties_value );
  on_property_value_changed();
}

template<typename T> void NumericPropertyWidget<T>::on_property_value_changed()
{
  m_spinbox->blockSignals(true);
  m_spinbox->set_values(NumericPropertyWidget<T>::get_properties_values());
  m_spinbox->blockSignals(true);
}

template class NumericPropertyWidget<int>;
template class NumericPropertyWidget<double>;

} // namespace omm