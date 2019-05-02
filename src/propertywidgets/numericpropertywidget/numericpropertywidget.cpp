#include "propertywidgets/numericpropertywidget/numericpropertywidget.h"

#include <QLineEdit>

// TODO units
// TODO alternative editor widgets (line edit, slider)

namespace omm
{

template<typename NumericPropertyT> NumericPropertyWidget<NumericPropertyT>
::NumericPropertyWidget(Scene& scene, const std::set<Property*>& properties)
  : PropertyWidget<NumericPropertyT>(scene, properties)
{
  using edit_type = NumericMultiValueEdit<typename NumericPropertyT::value_type>;
  auto spinbox = std::make_unique<edit_type>();
  QObject::connect(spinbox.get(), &AbstractNumericEdit::value_changed,
                   [this, spinbox=spinbox.get()]() {
    this->set_properties_value(spinbox->value());
  });
  m_spinbox = spinbox.get();
  this->set_default_layout(std::move(spinbox));

  using P = NumericPropertyT;
  const auto lower = Property::get_value<value_type, P>(properties, std::mem_fn(&P::lower));
  const auto upper = Property::get_value<value_type, P>(properties, std::mem_fn(&P::upper));
  m_spinbox->set_range(lower, upper);
  const auto mult = Property::get_value<double, P>(properties, std::mem_fn(&P::multiplier));
  m_spinbox->set_multiplier(mult);
  const auto step = Property::get_value<value_type, P>(properties, std::mem_fn(&P::step));
  m_spinbox->set_step(step);

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
