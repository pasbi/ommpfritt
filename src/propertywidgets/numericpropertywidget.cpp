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

template<typename NumericPropertyT>
class NumericPropertyWidget<NumericPropertyT>::SpinBox
  : public ::SpinBox<typename NumericPropertyT::value_type>
  , public MultiValueEdit<typename NumericPropertyT::value_type>
{
public:
  using value_type = typename NumericPropertyT::value_type;
  explicit SpinBox() {}
  void set_value(const value_type& value) override
  {
    ::SpinBox<value_type>::setValue(value);
  }

  value_type value() const override { return SpinBox::value(); }

protected:
  void set_inconsistent_value() override {
    NumericPropertyWidget<NumericPropertyT>::SpinBox::lineEdit()->setText("");
  }
};

template<typename NumericPropertyT> NumericPropertyWidget<NumericPropertyT>
::NumericPropertyWidget(Scene& scene, const Property::SetOfProperties& properties)
  : PropertyWidget<NumericPropertyT>(scene, properties)
{
  auto spinbox = std::make_unique<SpinBox>();
  m_spinbox = spinbox.get();
  this->set_default_layout(std::move(spinbox));

  const auto set_properties_value = [this](const value_type& value) {
    this->set_properties_value(value);
  };
  using value_changed_type = void(SpinBox::Base::*)(value_type);
  QObject::connect( m_spinbox,
                    static_cast<value_changed_type>(&SpinBox::Base::valueChanged),
                    set_properties_value );
  on_property_value_changed();
}

template<typename NumericPropertyT>
void NumericPropertyWidget<NumericPropertyT>::on_property_value_changed()
{
  m_spinbox->blockSignals(true);
  m_spinbox->set_values(NumericPropertyWidget<NumericPropertyT>::get_properties_values());
  m_spinbox->blockSignals(false);
}

template class NumericPropertyWidget<IntegerProperty>;
template class NumericPropertyWidget<FloatProperty>;

} // namespace omm