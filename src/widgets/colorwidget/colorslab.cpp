#include "widgets/colorwidget/colorslab.h"
#include "preferences/uicolors.h"
#include <QPainter>
#include <QPainterPath>

namespace omm
{
void ColorSlab::set_old_color(const Color& color)
{
  m_old_color = color;
  update();
}

void ColorSlab::set_new_color(const Color& color)
{
  m_new_color = color;
  update();
}

void ColorSlab::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  UiColors::draw_background(painter, rect());

  QPainterPath topleft;
  topleft.moveTo(QPoint(0, 0));
  topleft.lineTo(QPoint(width(), 0));
  topleft.lineTo(QPoint(width(), height()));
  topleft.closeSubpath();

  QPainterPath bottomright;
  bottomright.moveTo(QPoint(0, 0));
  bottomright.lineTo(QPoint(width(), height()));
  bottomright.lineTo(QPoint(0, height()));
  bottomright.closeSubpath();

  painter.fillPath(topleft, m_old_color.to_qcolor());
  painter.fillPath(bottomright, m_new_color.to_qcolor());
}

}  // namespace omm
