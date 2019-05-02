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
  using elem_type = typename value_type::element_type;
  VectorPropertyConfigWidget(QWidget* parent, Property& property)
    : PropertyConfigWidget<PropertyT>(parent, property)
  {
    enum class Dimension { X = 0, Y = 1 };

    auto& vector_property = type_cast<PropertyT&>(property);
    using namespace std::placeholders;
    auto [ min_x, max_x ] = NumericEdit<elem_type>::make_range_edits();
    QObject::connect(min_x.get(), &AbstractNumericEdit::value_changed,
                     [v=min_x.get(), &vector_property]() {
      vector_property.set_lower(value_type(v->value(), vector_property.upper_bound().y));
    });
    QObject:: connect(max_x.get(), &AbstractNumericEdit::value_changed,
                      [v=max_x.get(), &vector_property]() {
      vector_property.set_upper(value_type(v->value(), vector_property.upper_bound().y));
    });

    auto [ min_y, max_y ] = NumericEdit<elem_type>::make_range_edits();
    QObject::connect(min_y.get(), &AbstractNumericEdit::value_changed,
                     [v=min_y.get(), &vector_property]() {
      vector_property.set_lower(value_type(vector_property.upper_bound().x, v->value()));
    });
    QObject::connect(max_y.get(), &AbstractNumericEdit::value_changed,
                     [v=max_y.get(), &vector_property]() {
      vector_property.set_upper(value_type(vector_property.upper_bound().x, v->value()));
    });

    min_x->set_value(vector_property.lower().x);
    min_y->set_value(vector_property.lower().y);
    max_x->set_value(vector_property.upper().x);
    max_y->set_value(vector_property.upper().y);

    auto step_x_edit = std::make_unique<omm::NumericEdit<elem_type>>();
    step_x_edit->set_value(vector_property.step().x);
    QObject::connect(step_x_edit.get(), &AbstractNumericEdit::value_changed,
                     [&vector_property, e=step_x_edit.get()]() {
      vector_property.set_step(value_type(e->value(), vector_property.step().y));
    });

    auto step_y_edit = std::make_unique<omm::NumericEdit<elem_type>>();
    step_y_edit->set_value(vector_property.step().y);
    QObject::connect(step_y_edit.get(), &AbstractNumericEdit::value_changed,
                    [&vector_property, e=step_y_edit.get()]() {
      vector_property.set_step(value_type(vector_property.step().x, e->value()));
    });

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
