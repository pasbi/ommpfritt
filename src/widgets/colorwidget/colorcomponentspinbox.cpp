#include "widgets/colorwidget/colorcomponentspinbox.h"
#include "widgets/numericedit.h"
#include <QHBoxLayout>
#include <memory>

namespace omm
{
constexpr double R_FACTOR = 255.0;
constexpr double G_FACTOR = 255.0;
constexpr double B_FACTOR = 255.0;
constexpr double H_FACTOR = 255.0;
constexpr double S_FACTOR = 255.0;
constexpr double V_FACTOR = 255.0;
constexpr double A_FACTOR = 255.0;
const std::map<Color::Role, double> ColorComponentSpinBox::factor{
    {Color::Role::Red, R_FACTOR},
    {Color::Role::Green, G_FACTOR},
    {Color::Role::Blue, B_FACTOR},
    {Color::Role::Alpha, A_FACTOR},
    {Color::Role::Hue, H_FACTOR},
    {Color::Role::Saturation, S_FACTOR},
    {Color::Role::Value, V_FACTOR},
};

ColorComponentSpinBox::ColorComponentSpinBox(QWidget* parent) : AbstractColorComponentWidget(parent)
{
  auto layout = std::make_unique<QHBoxLayout>();
  auto spinbox = std::make_unique<NumericEdit<double>>();
  m_spin_box = spinbox.get();

  layout->addWidget(spinbox.release());
  setLayout(layout.release());

  connect(m_spin_box, &AbstractNumericEdit::value_changed, this, [this]() {
    Color color = this->color();
    color.set(role(), m_spin_box->value() / m_factor);
    set_color(color);
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
  m_factor = factor.at(role);
  m_spin_box->set_range(0.0, m_factor);
}

}  // namespace omm
