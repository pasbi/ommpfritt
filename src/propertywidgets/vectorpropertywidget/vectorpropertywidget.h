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
  explicit VectorPropertyWidget(Scene& scene, const std::set<Property*>& properties)
    : PropertyWidget<VectorPropertyT>(scene, properties)
  {
    const auto on_value_changed = [this](const value_type& value) {
      this->set_properties_value(value);
    };
    auto vector_edit = std::make_unique<VectorEdit<value_type>>(on_value_changed);
    m_vector_edit = vector_edit.get();
    this->set_default_layout(std::move(vector_edit));

    using P = VectorPropertyT;
    const auto lower = Property::get_value<value_type, P>(properties, std::mem_fn(&P::lower));
    const auto upper = Property::get_value<value_type, P>(properties, std::mem_fn(&P::upper));
    m_vector_edit->set_range(lower, upper);
    const auto mult = Property::get_value<double, P>(properties, std::mem_fn(&P::multiplier));
    m_vector_edit->set_multiplier(mult);
    const auto step = Property::get_value<value_type, P>(properties, std::mem_fn(&P::step));
    m_vector_edit->set_step(step);

    update_edit();
  }

protected:
  void update_edit() override
  {
    QSignalBlocker blocker(m_vector_edit);
    m_vector_edit->set_values(this->get_properties_values());
  }

private:
  VectorEdit<value_type>* m_vector_edit;
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
