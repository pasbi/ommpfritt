#pragma once

#include "widgets/colorwidget/abstractcolorcomponentwidget.h"

namespace omm
{
class ColorSlider : public AbstractColorComponentWidget
{
  Q_OBJECT
public:
  using AbstractColorComponentWidget::AbstractColorComponentWidget;
  void set_color(const Color& color) override;

protected:
  void paintEvent(QPaintEvent* e) override;
  void mousePressEvent(QMouseEvent* e) override;
  void mouseMoveEvent(QMouseEvent* e) override;

private:
  void set_value(int x);
};

}  // namespace omm
