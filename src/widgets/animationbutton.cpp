#include "widgets/animationbutton.h"

#include <QResizeEvent>
#include <QPainter>

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
  QWidget::paintEvent(event);

  if (m_state == State::NotAnimated) {
    QPen pen;
    pen.setColor(Qt::black);
    pen.setWidth(1);
    painter.setPen(pen);
    const int r = -4;
    painter.drawEllipse(rect().adjusted(r, r, r, r));
  } else if (m_state == State::Animated) {
    QPen pen;
    pen.setColor(Qt::red);
    pen.setWidth(3);
    painter.setPen(pen);
    const int r = -7;
    painter.drawEllipse(rect().adjusted(r, r, r, r));
  }
}

void AnimationButton::contextMenuEvent(QContextMenuEvent *event)
{

}

}  // namespace omm
