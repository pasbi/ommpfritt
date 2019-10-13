#include "managers/boundingboxmanager/anchorwidget.h"

#include <QMouseEvent>
#include <QPainter>
#include "logging.h"
#include <cmath>
#include <QTimer>
#include "preferences/uicolors.h"

namespace omm
{

AnchorWidget::AnchorWidget(QWidget *parent) : QWidget(parent)
{
  setMouseTracking(true);
}

AnchorWidget::Anchor AnchorWidget::anchor() const
{
  return m_value;
}

void AnchorWidget::set_anchor(const AnchorWidget::Anchor &anchor)
{
  if (m_value != anchor) {
    m_value = anchor;
    Q_EMIT anchor_changed(anchor);
  }
}

void AnchorWidget::mouseMoveEvent(QMouseEvent *event)
{
  m_mouse_pos = event->pos();
  QWidget::mouseMoveEvent(event);
  m_disable_hover = false;
  update();
}

void AnchorWidget::mousePressEvent(QMouseEvent *event)
{
  const QPointF pos = event->pos();
  const QRectF grid = anchor_grid();
  for (const Anchor anchor : PROPER_ANCHORS) {
    if (anchor_rect(anchor_position(grid, anchor)).contains(pos)) {
      if (m_value == anchor) {
        set_anchor(Anchor::None);
      } else {
        set_anchor(anchor);
      }
      update();
      m_disable_hover_for = anchor;
      QTimer::singleShot(1000, [this]() { m_disable_hover_for = Anchor::None; });
      break;
    }
  }
}

void AnchorWidget::enterEvent(QEvent *event)
{
  m_disable_hover = false;
  QWidget::enterEvent(event);
  update();
}

void AnchorWidget::leaveEvent(QEvent *event)
{
  m_disable_hover = true;
  QWidget::leaveEvent(event);
  update();
}

QSize AnchorWidget::minimumSizeHint() const
{
  return QSize(50, 50);
}

Vec2f AnchorWidget::anchor_position(const QRectF &grid) const
{
  return Vec2f(anchor_position(grid, anchor()));
}

void AnchorWidget::paintEvent(QPaintEvent*)
{
  const QRectF grid = anchor_grid();

  QPainter painter(this);
  painter.setRenderHint(QPainter::HighQualityAntialiasing);

  QPen pen;
  pen.setColor(ui_color(*this, QPalette::Text));
  pen.setWidth(2);
  pen.setCosmetic(true);
  painter.setPen(pen);
  painter.drawRect(grid);

  for (const Anchor anchor : PROPER_ANCHORS) {
    draw_anchor(painter, grid, anchor);
  }
}

void AnchorWidget::draw_anchor(QPainter& painter, const QRectF& grid, Anchor anchor) const
{
  const QPointF pos = anchor_position(grid, anchor);
  const bool is_selected = m_value == anchor;
  painter.save();
  if (std::isnan(pos.x()) || std::isnan(pos.y())) {
    return;
  }
  QRectF rect = anchor_rect(pos);
  QPen pen;
  pen.setColor(ui_color(*this, QPalette::Text));
  painter.setPen(pen);
  QBrush brush(Qt::SolidPattern);
  if (rect.contains(m_mouse_pos) && m_disable_hover_for != anchor && !m_disable_hover) {
    brush.setColor(ui_color(*this, "AnchorWidget", "hover"));
  } else if (is_selected) {
    brush.setColor(ui_color(*this, "AnchorWidget", "checked"));
  } else {
    brush.setColor(ui_color(*this, QPalette::Window));
  }
  painter.setBrush(brush);
  painter.drawEllipse(rect);
  painter.restore();
}

QRectF AnchorWidget::anchor_grid() const
{
  return QRectF(QPointF(ANCHOR_RADIUS, ANCHOR_RADIUS),
                QSizeF(width() - 2*ANCHOR_RADIUS, height() - 2*ANCHOR_RADIUS));
}

QRectF AnchorWidget::anchor_rect(const QPointF &pos) const
{
  return QRectF(QPointF(pos.x() - ANCHOR_RADIUS, pos.y() - ANCHOR_RADIUS),
                QSizeF(2*ANCHOR_RADIUS, 2*ANCHOR_RADIUS));
}

QPointF AnchorWidget::anchor_position(const QRectF &grid, const AnchorWidget::Anchor &anchor)
{
  switch (anchor) {
  case Anchor::TopLeft:
    return grid.topLeft();
  case Anchor::TopRight:
    return grid.topRight();
  case Anchor::BottomLeft:
    return grid.bottomLeft();
  case Anchor::BottomRight:
    return grid.bottomRight();
  case Anchor::Center:
    return grid.center();
  case Anchor::None:
  default:
    return QPointF(std::numeric_limits<double>::quiet_NaN(),
                   std::numeric_limits<double>::quiet_NaN());
  }
}


}  // namespace omm
