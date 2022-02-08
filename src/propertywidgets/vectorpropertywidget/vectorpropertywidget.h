#pragma once

#include "propertywidgets/numericpropertywidget/numericmultivalueedit.h"
#include "propertywidgets/propertywidget.h"
#include <QHBoxLayout>
#include <QLabel>

namespace omm
{
template<typename VectorPropertyT>
class VectorPropertyWidget : public PropertyWidget<VectorPropertyT>
{
public:
  using value_type = typename VectorPropertyT::value_type;
  using elem_type = typename value_type::element_type;
  explicit VectorPropertyWidget(Scene& scene, const std::set<Property*>& properties)
      : PropertyWidget<VectorPropertyT>(scene, properties)
  {
    auto x_edit = std::make_unique<NumericMultiValueEdit<elem_type>>();
    m_x_edit = x_edit.get();
    QObject::connect(m_x_edit, &NumericMultiValueEdit<elem_type>::value_changed, [this]() {
      set_properties_value<0>(m_x_edit->value());
    });

    auto y_edit = std::make_unique<NumericMultiValueEdit<elem_type>>();
    m_y_edit = y_edit.get();
    QObject::connect(m_y_edit, &NumericMultiValueEdit<elem_type>::value_changed, [this]() {
      set_properties_value<1>(m_y_edit->value());
    });

    const auto step = this->template configuration<value_type>(NumericPropertyDetail::STEP_POINTER);
    x_edit->set_step(step.x);
    y_edit->set_step(step.y);

    const auto mult
        = this->template configuration<double>(NumericPropertyDetail::MULTIPLIER_POINTER);
    x_edit->set_multiplier(mult);
    y_edit->set_multiplier(mult);

    const auto lower
        = this->template configuration<value_type>(NumericPropertyDetail::LOWER_VALUE_POINTER);
    const auto upper
        = this->template configuration<value_type>(NumericPropertyDetail::UPPER_VALUE_POINTER);
    x_edit->set_range(lower.x, upper.x);
    y_edit->set_range(lower.y, upper.y);

    auto layout = std::make_unique<QHBoxLayout>();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(x_edit.release());
    layout->addWidget(y_edit.release());

    auto container = std::make_unique<QWidget>();
    container->setLayout(layout.release());

    auto vlayout = std::make_unique<AbstractPropertyWidget::LabelLayout>();
    vlayout->set_label(this->label());
    vlayout->set_thing(std::move(container));

    this->setLayout(vlayout.release());

    update_edit();
  }

protected:
  void update_edit() override
  {
    QSignalBlocker x_blocker(m_x_edit);
    QSignalBlocker y_blocker(m_y_edit);
    const auto values = this->get_properties_values();
    m_x_edit->set_values(util::transform(values, [](const auto& vec) { return vec.x; }));
    m_y_edit->set_values(util::transform(values, [](const auto& vec) { return vec.y; }));
  }

  template<std::size_t dim> void set_properties_value(const elem_type& value)
  {
    const auto properties = this->properties();

    const auto is_noop
        = [&value](const Property* p) { return p->value<value_type>()[dim] == value; };

    if (!std::all_of(properties.begin(), properties.end(), is_noop)) {
      using command_t = VectorPropertiesCommand<VectorPropertyT, dim>;
      auto command = std::make_unique<command_t>(properties, value);

      this->submit_command(std::move(command));
    }
  }

private:
  NumericMultiValueEdit<elem_type>* m_x_edit;
  NumericMultiValueEdit<elem_type>* m_y_edit;
};

}  // namespace omm
