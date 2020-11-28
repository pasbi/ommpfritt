#pragma once

#include "widgets/colorwidget/abstractcolorcomponentwidget.h"

namespace omm
{
template<typename T> class NumericEdit;

class ColorComponentSpinBox : public AbstractColorComponentWidget
{
  Q_OBJECT
public:
  explicit ColorComponentSpinBox(QWidget* parent = nullptr);
  void set_color(const Color& color) override;

  static const std::map<Color::Role, double> factor;
  void set_role(Color::Role role) override;

private:
  NumericEdit<double>* m_spin_box;
  double m_factor = 1.0;
};

}  // namespace omm
