#include "widgets/colorwidget/colorcomponentspinbox.h"
#include "widgets/numericedit.h"
#include <memory>
#include <QHBoxLayout>

namespace omm
{

std::map<Color::Role, double> ColorComponentSpinBox::factor {
  { Color::Role::Red,        255.0 },
  { Color::Role::Green,      255.0 },
  { Color::Role::Blue,       255.0 },
  { Color::Role::Alpha,      100.0 },
  { Color::Role::Hue,        360.0 },
  { Color::Role::Saturation, 100.0 },
  { Color::Role::Value,      100.0 },
};

ColorComponentSpinBox::ColorComponentSpinBox(QWidget* parent) : AbstractColorComponentWidget(parent)
{
  auto layout = std::make_unique<QHBoxLayout>();
  auto spinbox = std::make_unique<NumericEdit<double>>();
  m_spin_box = spinbox.get();

  layout->addWidget(spinbox.release());
  setLayout(layout.release());

  connect(m_spin_box, &AbstractNumericEdit::value_changed, this, [this]() {
     set_color(color().set(role(), m_spin_box->value() / m_factor));
  });
}

void ColorComponentSpinBox::set_color(const Color& color)
{
  AbstractColorComponentWidget::set_color(color);
  m_spin_box->set_value(m_factor * color.get(role()));
}

void ColorComponentSpinBox::set_role(Color::Role role)
{
  AbstractColorComponentWidget::set_role(role);
  m_factor = factor[role];
  m_spin_box->set_range(0.0, m_factor);
}

}  // namespace
