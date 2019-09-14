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
  using value_type = typename NumericPropertyT::value_type;
  using edit_type = NumericMultiValueEdit<value_type>;
  auto spinbox = std::make_unique<edit_type>();
  QObject::connect(spinbox.get(), &AbstractNumericEdit::value_changed,
                   [this, spinbox=spinbox.get()]() {
    this->set_properties_value(spinbox->value());
  });
  m_spinbox = spinbox.get();
  this->set_default_layout(std::move(spinbox));

  using P = NumericPropertyDetail;
  const auto lower = this->template configuration<value_type>(P::LOWER_VALUE_POINTER);
  const auto upper = this->template configuration<value_type>(P::UPPER_VALUE_POINTER);

  const auto mult = this->template configuration<double>(P::MULTIPLIER_POINTER);
  const auto step = this->template configuration<value_type>(P::STEP_POINTER);
  m_spinbox->set_range(lower, upper);
  m_spinbox->set_multiplier(mult);
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
