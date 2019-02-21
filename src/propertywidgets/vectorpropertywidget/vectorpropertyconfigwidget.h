#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "properties/vectorproperty.h"
#include "widgets/numericedit.h"
#include <memory>
#include <QHBoxLayout>
#include <functional>

template<typename elem_type>
auto make_vector_edit(std::unique_ptr<QWidget> x, std::unique_ptr<QWidget> y)
{
  auto layout = std::make_unique<QHBoxLayout>();
  layout->addWidget(x.release());
  layout->addWidget(y.release());
  return layout;
}

namespace omm
{

template<typename PropertyT>
class VectorPropertyConfigWidget : public PropertyConfigWidget<PropertyT>
{
public:
  using value_type = typename PropertyT::value_type;
  using elem_type = typename value_type::value_type;
  VectorPropertyConfigWidget(QWidget* parent, Property& property)
    : PropertyConfigWidget<PropertyT>(parent, property)
  {
    enum class Dimension { X = 0, Y = 1 };

    auto& vector_property = type_cast<PropertyT&>(property);
    const auto on_range_changed = [&vector_property]
                                    (const Dimension d, const elem_type min, const elem_type max)
    {
      auto min2d = vector_property.lower();
      auto max2d = vector_property.upper();
      min2d(static_cast<int>(d)) = min;
      max2d(static_cast<int>(d)) = max;
      vector_property.set_range(min2d, max2d);
    };

    using namespace std::placeholders;
    const auto on_x_range_changed = [on_range_changed](const elem_type min, const elem_type max) {
      return on_range_changed(Dimension::X, min, max);
    };
    auto [ min_x, max_x ] = NumericEdit<elem_type>::make_range_edits(on_x_range_changed);
    const auto on_y_range_changed = [on_range_changed](const elem_type min, const elem_type max) {
      return on_range_changed(Dimension::Y, min, max);
    };
    auto [ min_y, max_y ] = NumericEdit<elem_type>::make_range_edits(on_y_range_changed);

    min_x->set_value(vector_property.lower()(0));
    min_y->set_value(vector_property.lower()(1));
    max_x->set_value(vector_property.upper()(0));
    max_y->set_value(vector_property.upper()(1));

    const auto make_step_edit = [&vector_property](const Dimension dim) {
      const auto on_step_changed = [&vector_property, dim](const elem_type step) {
        auto step2d = vector_property.step();
        step2d(static_cast<int>(dim)) = step;
        vector_property.set_step(step2d);
      };
      auto step_edit = std::make_unique<omm::NumericEdit<elem_type>>(on_step_changed);
      step_edit->set_lower(NumericEditDetail::smallest_step<elem_type>);
      return step_edit;
    };

    auto step_x_edit = make_step_edit(Dimension::X);
    step_x_edit->set_value(vector_property.step()(0));
    auto step_y_edit = make_step_edit(Dimension::Y);
    step_y_edit->set_value(vector_property.step()(1));

    auto min_layout = make_vector_edit<elem_type>(std::move(min_x), std::move(min_y));
    auto max_layout = make_vector_edit<elem_type>(std::move(max_x), std::move(max_y));
    auto step_layout = make_vector_edit<elem_type>(std::move(step_x_edit), std::move(step_y_edit));
    this->form_layout()->addRow(QObject::tr("min", "NumericProperty"), min_layout.release());
    this->form_layout()->addRow(QObject::tr("max", "NumericProperty"), max_layout.release());
    this->form_layout()->addRow(QObject::tr("step", "NumericProperty"), step_layout.release());
  }
};


class IntegerVectorPropertyConfigWidget : public VectorPropertyConfigWidget<IntegerVectorProperty>
{
  Q_OBJECT
public:
  using VectorPropertyConfigWidget::VectorPropertyConfigWidget;
  std::string type() const override;
};

class FloatVectorPropertyConfigWidget : public VectorPropertyConfigWidget<FloatVectorProperty>
{
  Q_OBJECT
public:
  using VectorPropertyConfigWidget::VectorPropertyConfigWidget;
  std::string type() const override;
};

}  // namespace omm
