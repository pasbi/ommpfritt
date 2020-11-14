#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "widgets/numericedit.h"
#include <memory>
#include <QHBoxLayout>
#include <functional>
#include <QFormLayout>

namespace omm
{

template<typename PropertyT>
class VectorPropertyConfigWidget : public PropertyConfigWidget<PropertyT>
{
public:
  using T = typename PropertyT::value_type;
  using ElementT = typename T::element_type;
  explicit VectorPropertyConfigWidget()
  {
    for (QString d : { "x", "y" }) {
      auto [ min_edit, max_edit ] = NumericEdit<ElementT>::make_range_edits();
      // ownership is only temporarily passed to this.
      m_edits[d + NumericPropertyDetail::LOWER_VALUE_POINTER] = min_edit.release();

      // ownership is only temporarily passed to this.
      m_edits[d + NumericPropertyDetail::UPPER_VALUE_POINTER] = max_edit.release();
      auto step_edit = std::make_unique<NumericEdit<ElementT>>();
      step_edit->set_lower(NumericProperty<ElementT>::smallest_step());

      // ownership is only temporarily passed to this.
      m_edits[d + NumericPropertyDetail::STEP_POINTER] = step_edit.release();
    }

    auto mult_edit = std::make_unique<NumericEdit<double>>();
    // ownership is only temporarily passed to this.
    m_edits[NumericPropertyDetail::MULTIPLIER_POINTER] = mult_edit.release();

    auto layout = std::make_unique<QFormLayout>();
    static const std::vector keys = {  NumericPropertyDetail::LOWER_VALUE_POINTER,
                                       NumericPropertyDetail::UPPER_VALUE_POINTER,
                                       NumericPropertyDetail::STEP_POINTER };
    for (QString k : keys) {
      auto pair_layout = std::make_unique<QHBoxLayout>();
      for (QString d : { "x", "y" }) {
        pair_layout->addWidget(m_edits[d + k]);  // pass ownership from this to layout
      }
      layout->addRow(QObject::tr(k.toUtf8().constData(),
                                 "NumericProperty"), pair_layout.release());
      layout->addRow(QObject::tr(NumericPropertyDetail::MULTIPLIER_POINTER, "NumericProperty"),
                     m_edits[NumericPropertyDetail::MULTIPLIER_POINTER]);
    }
    this->setLayout(layout.release());
  }

  void init(const Property::Configuration &configuration) override
  {
    const ElementT llower = NumericProperty<ElementT>::lowest_possible_value();
    const ElementT uupper = NumericProperty<ElementT>::highest_possible_value();

    const T lower = configuration.get(NumericPropertyDetail::LOWER_VALUE_POINTER, T(llower, llower));
    const T upper = configuration.get(NumericPropertyDetail::UPPER_VALUE_POINTER, T(uupper, uupper));
    const T step = configuration.get(NumericPropertyDetail::STEP_POINTER, T(1, 1));
    const double mult = configuration.get(NumericPropertyDetail::MULTIPLIER_POINTER, 1.0);

    for (auto&& [i, d] : std::vector<std::pair<int, QString>>{ {0, "x"}, {1, "y"} }) {
      static_cast<NumericEdit<ElementT>*>(m_edits[d + NumericPropertyDetail::LOWER_VALUE_POINTER])->set_range(llower, upper[i]);
      static_cast<NumericEdit<ElementT>*>(m_edits[d + NumericPropertyDetail::LOWER_VALUE_POINTER])->set_value(lower[i]);
      static_cast<NumericEdit<ElementT>*>(m_edits[d + NumericPropertyDetail::UPPER_VALUE_POINTER])->set_range(lower[i], uupper);
      static_cast<NumericEdit<ElementT>*>(m_edits[d + NumericPropertyDetail::UPPER_VALUE_POINTER])->set_value(upper[i]);
      static_cast<NumericEdit<ElementT>*>(m_edits[d + NumericPropertyDetail::STEP_POINTER])->set_lower(NumericProperty<ElementT>::smallest_step());
      static_cast<NumericEdit<ElementT>*>(m_edits[d + NumericPropertyDetail::STEP_POINTER])->set_value(step[i]);
    }
    static_cast<NumericEdit<double>*>(m_edits[NumericPropertyDetail::MULTIPLIER_POINTER])->set_value(mult);
  }

  void update(Property::Configuration &configuration) const override
  {
    for (const QString& key : { NumericPropertyDetail::LOWER_VALUE_POINTER,
                                    NumericPropertyDetail::UPPER_VALUE_POINTER,
                                    NumericPropertyDetail::STEP_POINTER })
    {
      const auto* x_edit = static_cast<NumericEdit<ElementT>*>(m_edits.at("x" + key));
      const auto* y_edit = static_cast<NumericEdit<ElementT>*>(m_edits.at("y" + key));
      configuration[key] = T(x_edit->value(), y_edit->value());
      configuration[NumericPropertyDetail::MULTIPLIER_POINTER]
          = static_cast<NumericEdit<double>*>(m_edits.at(NumericPropertyDetail::MULTIPLIER_POINTER))->value();
    }
  }

private:
  std::map<QString, AbstractNumericEdit*> m_edits;
};

}  // namespace omm
