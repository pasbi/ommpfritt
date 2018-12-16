#include "propertywidgets/numericpropertywidget/numericpropertywidget.h"

#include <QSpinBox>
#include <QLineEdit>

// TODO units
// TODO alternative editor widgets (line edit, slider)
// TODO min, max, step, default

namespace omm
{

template<typename NumericPropertyT> NumericPropertyWidget<NumericPropertyT>
::NumericPropertyWidget(Scene& scene, const Property::SetOfProperties& properties)
  : PropertyWidget<NumericPropertyT>(scene, properties)
{
  using SpinBoxT = SpinBox<typename NumericPropertyT::value_type>;
  auto spinbox = std::make_unique<SpinBoxT>();
  m_spinbox = spinbox.get();
  this->set_default_layout(std::move(spinbox));

  const auto set_properties_value = [this](const value_type& value) {
    this->set_properties_value(value);
  };
  using value_changed_type = void(SpinBoxT::Base::*)(value_type);
  QObject::connect( m_spinbox,
                    static_cast<value_changed_type>(&SpinBoxT::Base::valueChanged),
                    set_properties_value );
  update_edit();
}

template<typename NumericPropertyT>
void NumericPropertyWidget<NumericPropertyT>::update_edit()
{
  QSignalBlocker blocker(m_spinbox);
  m_spinbox->set_values(NumericPropertyWidget<NumericPropertyT>::get_properties_values());
}

template class NumericPropertyWidget<IntegerProperty>;
template class NumericPropertyWidget<FloatProperty>;

} // namespace omm