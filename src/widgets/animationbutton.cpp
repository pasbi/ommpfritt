#include "widgets/animationbutton.h"

#include <QResizeEvent>
#include <QPainter>
#include <cmath>
#include "logging.h"

namespace omm
{

AnimationButton::AnimationButton(QWidget *parent) : QWidget(parent)
{
  setContextMenuPolicy(Qt::DefaultContextMenu);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void AnimationButton::set_state(AnimationButton::State state)
{
  m_state = state;
  update();
}

void AnimationButton::resizeEvent(QResizeEvent *event)
{
  setFixedWidth(event->size().height());
  QWidget::resizeEvent(event);
}

void AnimationButton::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::HighQualityAntialiasing);
  QWidget::paintEvent(event);

  const QRectF rect = this->rect();
  auto centered = [rect](const double relative_radius) {
    const QSizeF absolute_radius = relative_radius * rect.size() / 2.0;
    const QPointF tl = rect.center() - QPointF(absolute_radius.width(), absolute_radius.height());
    return QRectF(tl, 2*absolute_radius);
  };
  const double pen_width_base = std::min(rect.width(), rect.height());

  QPen pen;
  switch (m_state) {
  case State::NotAnimated:
    pen.setColor(Qt::black);
    pen.setWidthF(pen_width_base * 0.2);
    painter.setPen(pen);
    painter.drawEllipse(centered(0.8));
    break;
  case State::Animated:
    pen.setColor(Qt::red);
    pen.setWidthF(pen_width_base * 0.2);
    painter.setPen(pen);
    painter.drawEllipse(centered(0.8));
    break;
  case State::KeyValue:
    pen.setColor(Qt::red);
    pen.setWidthF(pen_width_base * 0.2);
    painter.setPen(pen);
    painter.drawEllipse(centered(0.8));
    QPainterPath ellipse;
    ellipse.addEllipse(centered(0.4));
    painter.fillPath(ellipse, Qt::red);
    break;
  }
}

void AnimationButton::contextMenuEvent(QContextMenuEvent *event)
{

}

void AnimationButton::mousePressEvent(QMouseEvent *event)
{
  switch (m_state) {
  case State::Animated:
    return set_state(State::NotAnimated);
  case State::KeyValue:
    return set_state(State::Animated);
  case State::NotAnimated:
    return set_state(State::KeyValue);
  }
  QWidget::mousePressEvent(event);
}

}  // namespace omm
