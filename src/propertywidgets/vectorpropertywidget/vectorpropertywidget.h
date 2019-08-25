#pragma once

#include "propertywidgets/propertywidget.h"
#include "properties/vectorproperty.h"
#include "propertywidgets/numericpropertywidget/numericmultivalueedit.h"
#include <QHBoxLayout>

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
    m_x_edit = std::make_unique<NumericMultiValueEdit<elem_type>>().release();
    QObject::connect(m_x_edit, &NumericMultiValueEdit<elem_type>::value_changed, [this]() {
      set_properties_value<0>(m_x_edit->value());
    });

    m_y_edit = std::make_unique<NumericMultiValueEdit<elem_type>>().release();
    QObject::connect(m_y_edit, &NumericMultiValueEdit<elem_type>::value_changed, [this]() {
      set_properties_value<1>(m_y_edit->value());
    });

    const auto get_step = std::mem_fn(&VectorPropertyT::step);
    const auto step = Property::get_value<value_type, VectorPropertyT>(properties, get_step);
    m_x_edit->set_step(step.x);
    m_y_edit->set_step(step.y);

    const auto get_mult = std::mem_fn(&VectorPropertyT::multiplier);
    const double multiplier = Property::get_value<double, VectorPropertyT>(properties, get_mult);
    m_x_edit->set_multiplier(multiplier);
    m_y_edit->set_multiplier(multiplier);

    const auto get_lower = std::mem_fn(&VectorPropertyT::lower);
    const auto lower = Property::get_value<value_type, VectorPropertyT>(properties, get_lower);
    const auto get_upper = std::mem_fn(&VectorPropertyT::upper);
    const auto upper = Property::get_value<value_type, VectorPropertyT>(properties, get_upper);
    m_x_edit->set_range(lower.x, upper.x);
    m_y_edit->set_range(lower.y, upper.y);

    auto container = std::make_unique<QWidget>(this);
    auto* layout = std::make_unique<QHBoxLayout>(container.get()).release();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_x_edit);
    layout->addWidget(m_y_edit);
    this->set_default_layout(std::move(container));

    update_edit();
  }

protected:
  void update_edit() override
  {
    QSignalBlocker x_blocker(m_x_edit);
    QSignalBlocker y_blocker(m_y_edit);
    const auto values = this->get_properties_values();
    m_x_edit->set_values(::transform<elem_type>(values, [](const auto& vec) { return vec.x; }));
    m_y_edit->set_values(::transform<elem_type>(values, [](const auto& vec) { return vec.y; }));
  }

  template<std::size_t dim> void set_properties_value(const elem_type& value)
  {
    const auto properties = this->properties();

    const auto is_noop = [&value](const Property* p) {
      return p->value<value_type>()[dim] == value;
    };

    if (!std::all_of(properties.begin(), properties.end(), is_noop)) {
      const bool wrap = std::any_of(properties.begin(), properties.end(), [](const Property* p) {
        return p->wrap_with_macro;
      });

      using command_t = VectorPropertiesCommand<VectorPropertyT, dim>;
      auto command = std::make_unique<command_t>(properties, value);

      std::unique_ptr<Macro> macro;
      if (wrap) {
        macro = this->scene.history.start_macro(QString::fromStdString(command->label()));
        for (auto* property : properties) { property->pre_submit(*property); }
      }
      this->scene.submit(std::move(command));
      if (wrap) {
        for (auto* property : properties) { property->post_submit(*property); }
      }
    }
  }

private:
  NumericMultiValueEdit<elem_type>* m_x_edit;
  NumericMultiValueEdit<elem_type>* m_y_edit;
};

class IntegerVectorPropertyWidget : public VectorPropertyWidget<IntegerVectorProperty>
{
public:
  using VectorPropertyWidget::VectorPropertyWidget;
  std::string type() const override { return TYPE; }
};

class FloatVectorPropertyWidget : public VectorPropertyWidget<FloatVectorProperty>
{
public:
  using VectorPropertyWidget::VectorPropertyWidget;
  std::string type() const override { return TYPE; }
};



}  // namespace omm
