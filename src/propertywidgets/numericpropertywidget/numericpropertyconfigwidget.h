#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "properties/integerproperty.h"
#include "properties/floatproperty.h"

#include <QHBoxLayout>
#include <QFormLayout>
#include "widgets/numericedit.h"

namespace omm
{

template<typename PropertyT>
class NumericPropertyConfigWidget : public PropertyConfigWidget<PropertyT>
{
public:
  using T = typename PropertyT::value_type;
  NumericPropertyConfigWidget()
  {
    auto [ min_edit, max_edit ] = NumericEdit<T>::make_range_edits();
    m_min_edit = min_edit.get();
    m_max_edit = max_edit.get();
    auto step_edit = std::make_unique<NumericEdit<T>>();
    m_step_edit = step_edit.get();
    m_step_edit->set_lower(NumericProperty<T>::smallest_step);
    auto mult_edit = std::make_unique<NumericEdit<double>>();
    m_mult_edit = mult_edit.get();

    auto layout = std::make_unique<QFormLayout>();
    layout->addRow(QObject::tr(NumericPropertyDetail::LOWER_VALUE_POINTER, "NumericProperty"), min_edit.release());
    layout->addRow(QObject::tr(NumericPropertyDetail::UPPER_VALUE_POINTER, "NumericProperty"), max_edit.release());
    layout->addRow(QObject::tr(NumericPropertyDetail::STEP_POINTER, "NumericProperty"), step_edit.release());
    layout->addRow(QObject::tr(NumericPropertyDetail::MULTIPLIER_POINTER, "NumericProperty"), mult_edit.release());
    this->setLayout(layout.release());
  }

  void init(const Property::Configuration &configuration) override
  {
    const auto llower =  NumericProperty<T>::lowest_possible_value;
    const auto uupper = NumericProperty<T>::highest_possible_value;
    const auto lower = configuration.get<T>(NumericPropertyDetail::LOWER_VALUE_POINTER, llower);
    const auto upper = configuration.get<T>(NumericPropertyDetail::UPPER_VALUE_POINTER, uupper);
    const auto step = configuration.get<T>(NumericPropertyDetail::STEP_POINTER, T(1));
    const auto mult = configuration.get<double>(NumericPropertyDetail::MULTIPLIER_POINTER, 1.0);

    m_min_edit->set_range(llower, upper);
    m_min_edit->set_value(lower);
    m_max_edit->set_range(lower, uupper);
    m_max_edit->set_value(upper);
    m_step_edit->set_lower(NumericProperty<T>::smallest_step);
    m_step_edit->set_value(step);
    m_mult_edit->set_value(mult);
  }

  void update(Property::Configuration &configuration) const override
  {
    configuration[NumericPropertyDetail::LOWER_VALUE_POINTER] = m_min_edit->value();
    configuration[NumericPropertyDetail::UPPER_VALUE_POINTER] = m_max_edit->value();
    configuration[NumericPropertyDetail::STEP_POINTER] = m_step_edit->value();
    configuration[NumericPropertyDetail::MULTIPLIER_POINTER] = m_mult_edit->value();
  }

private:
  NumericEdit<T>* m_min_edit;
  NumericEdit<T>* m_max_edit;
  NumericEdit<T>* m_step_edit;
  NumericEdit<double>* m_mult_edit;
};

}  // namespace omm
