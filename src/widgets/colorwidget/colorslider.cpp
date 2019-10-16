#include "widgets/colorwidget/colorslider.h"

#include <QMouseEvent>
#include <QPainter>
#include "preferences/uicolors.h"

namespace omm
{

void ColorSlider::set_color(const Color& color)
{
  AbstractColorComponentWidget::set_color(color);
  update();
}

void ColorSlider::paintEvent(QPaintEvent *e)
{
  Q_UNUSED(e)

  static constexpr auto hue_steps = { 0.0, 1/6.0, 2/6.0, 3/6.0, 4/6.0, 5/6.0, 1.0 };
  static constexpr auto simple_steps = { 0.0, 1.0 };
  const auto steps = role() == Color::Role::Hue ? hue_steps : simple_steps;

  QLinearGradient gradient;
  gradient.setStart(QPoint());
  gradient.setFinalStop(QPointF(width(), 0));
  for (double d : steps) {
    gradient.setColorAt(d, color().set(role(), d).to_qcolor());
    gradient.setColorAt(d, color().set(role(), d).to_qcolor());
  }
  QPainter painter(this);
  UiColors::draw_background(painter, rect());
  painter.fillRect(rect(), gradient);
  painter.setRenderHint(QPainter::HighQualityAntialiasing);

  painter.translate(color().get(role()) * width(), 0);
  QPainterPath path;
  path.moveTo(0.0, height() / 2.0);
  path.lineTo(5.0, height());
  path.lineTo(-5.0, height());
  path.closeSubpath();
  painter.fillPath(path, ui_color(*this, QPalette::Base));
  QPen pen;
  pen.setWidth(2.0);
  pen.setColor(ui_color(*this, QPalette::Text));
  painter.setPen(pen);
  painter.drawPath(path);
}

void ColorSlider::mousePressEvent(QMouseEvent* e)
{
  set_value(e->x());
}

void ColorSlider::mouseMoveEvent(QMouseEvent* e)
{
  set_value(e->x());
}

void ColorSlider::set_value(int x)
{
  const double d = static_cast<double>(x) / static_cast<double>(width());
  set_color(color().set(role(), std::clamp(d, 0.0, 1.0)));
}

}  // namespace omm
