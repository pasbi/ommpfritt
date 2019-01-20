#pragma once

#include "propertywidgets/abstractedit.h"
#include <armadillo>
#include <QHBoxLayout>
#include "propertywidgets/multivalueedit.h"
#include "properties/vectorproperty.h"
#include "../numericpropertywidget/spinbox.h"

namespace omm
{

template<typename T> class VectorEdit
  : public QWidget, public MultiValueEdit<T>
{
public:
  using value_type = T;
  using elem_type = std::conditional_t< std::is_same_v<typename value_type::elem_type, long long>,
                                        int,
                                        double >;
  explicit VectorEdit(const std::function<void(const value_type&)>& on_value_changed)
    : MultiValueEdit<T>(on_value_changed)
  {
    std::make_unique<QHBoxLayout>(this).release();

    m_x_spinbox = std::make_unique<SpinBox<elem_type>>([this](const auto& value) {
      set_value(value_type{ this->value()(0), value });
    }).release();
    m_y_spinbox = std::make_unique<SpinBox<elem_type>>([this](const auto& value) {
      set_value(value_type{ value, this->value()(1) });
    }).release();

    layout()->addWidget(m_x_spinbox);
    layout()->addWidget(m_y_spinbox);
  }

  void set_value(const value_type& value) override
  {
    if (value != this->value()) {
      m_x_spinbox->set_value(value(0));
      m_y_spinbox->set_value(value(1));
      this->on_value_changed(value);
      update();
    }
  }

  value_type value() const override
  {
    return value_type{m_x_spinbox->value(), m_y_spinbox->value()};
  }

protected:
  void set_inconsistent_value() override
  {
    m_x_spinbox->set_inconsistent_value();
    m_y_spinbox->set_inconsistent_value();
  }

private:
  bool m_is_consistent = false;
  arma::vec2 m_current_value{0.0, 0.0};

  SpinBox<elem_type>* m_x_spinbox;
  SpinBox<elem_type>* m_y_spinbox;
};

}  // namespace omm
