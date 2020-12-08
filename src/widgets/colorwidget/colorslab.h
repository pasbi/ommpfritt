#pragma once

#include "color/color.h"
#include <QWidget>

namespace omm
{
class ColorSlab : public QWidget
{
public:
  using QWidget::QWidget;

public:
  void set_old_color(const Color& color);
  void set_new_color(const Color& color);

protected:
  void paintEvent(QPaintEvent*) override;

private:
  Color m_old_color;
  Color m_new_color;
};

}  // namespace omm
