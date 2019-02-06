#pragma once

#include "propertywidgets/propertywidget.h"
#include "properties/vectorproperty.h"
#include "propertywidgets/vectorpropertywidget/vectoredit.h"

namespace omm
{

template<typename VectorPropertyT>
class VectorPropertyWidget : public PropertyWidget<VectorPropertyT>
{
public:
  using value_type = typename VectorPropertyT::value_type;
  using elem_type = typename value_type::elem_type;
  explicit VectorPropertyWidget(Scene& scene, const std::set<Property*>& properties)
    : PropertyWidget<VectorPropertyT>(scene, properties)
  {
    m_x_edit = std::make_unique<NumericMultiValueEdit<elem_type>>([this](const elem_type value) {
      set_properties_value<0>(value);
    }).release();

    m_y_edit = std::make_unique<NumericMultiValueEdit<elem_type>>([this](const elem_type value) {
      set_properties_value<1>(value);
    }).release();

    const auto get_step = std::mem_fn(&VectorPropertyT::step);
    const auto step = Property::get_value<value_type, VectorPropertyT>(properties, get_step);
    m_x_edit->set_step(step(0));
    m_y_edit->set_step(step(1));

    const auto get_mult = std::mem_fn(&VectorPropertyT::multiplier);
    const double multiplier = Property::get_value<double, VectorPropertyT>(properties, get_mult);
    m_x_edit->set_multiplier(multiplier);
    m_y_edit->set_multiplier(multiplier);

    const auto get_lower = std::mem_fn(&VectorPropertyT::lower);
    const auto lower = Property::get_value<value_type, VectorPropertyT>(properties, get_lower);
    const auto get_upper = std::mem_fn(&VectorPropertyT::upper);
    const auto upper = Property::get_value<value_type, VectorPropertyT>(properties, get_upper);
    m_x_edit->set_range(lower(0), upper(0));
    m_y_edit->set_range(lower(1), upper(1));

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
    m_x_edit->set_values(::transform<elem_type>(values, [](const auto& vec) { return vec(0); }));
    m_y_edit->set_values(::transform<elem_type>(values, [](const auto& vec) { return vec(1); }));
  }

  template<std::size_t dim> void set_properties_value(const elem_type& value)
  {
    const auto properties = this->properties();

    const auto is_noop = [&value](const Property* p) {
      return p->value<value_type>()(dim) == value;
    };

    if (!std::all_of(properties.begin(), properties.end(), is_noop)) {
      const bool wrap = std::any_of(properties.begin(), properties.end(), [](const Property* p) {
        return p->wrap_with_macro;
      });

      using command_t = VectorPropertiesCommand<VectorPropertyT, dim>;
      auto command = std::make_unique<command_t>(properties, value);

      if (wrap) {
        this->scene.undo_stack.beginMacro(QString::fromStdString(command->label()));
        for (auto* property : properties) { property->pre_submit(*property); }
      }
      this->scene.submit(std::move(command));
      if (wrap) {
        for (auto* property : properties) { property->post_submit(*property); }
        this->scene.undo_stack.endMacro();
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
