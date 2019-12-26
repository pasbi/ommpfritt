#include "mainwindow/viewport/anchorhud.h"
#include "mainwindow/application.h"
#include <cmath>
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>
#include "preferences/uicolors.h"

namespace
{

QRectF anchor_rect(const QPointF &pos)
{
  constexpr auto ANCHOR_RADIUS = omm::AnchorHUD::ANCHOR_RADIUS;
  return QRectF(QPointF(pos.x() - ANCHOR_RADIUS, pos.y() - ANCHOR_RADIUS),
                QSizeF(2*ANCHOR_RADIUS, 2*ANCHOR_RADIUS));
}

}  // namespace

namespace omm
{

AnchorHUD::AnchorHUD(QWidget& widget)
  : m_widget(widget), m_anchor(Application::instance().options().anchor())

{
  connect(&Application::instance().options(), SIGNAL(anchor_changed(Anchor)),
          this, SLOT(set_anchor(Anchor)));
  connect(this, SIGNAL(anchor_changed(Anchor)),
          &Application::instance().options(), SLOT(set_anchor(Anchor)));
}

QSize AnchorHUD::size() const
{
  return QSize(50, 50);
}

void AnchorHUD::draw(QPainter& painter) const
{
  const QRectF grid = anchor_grid();

  painter.setRenderHint(QPainter::Antialiasing);

  painter.save();
  QPen pen;
  pen.setColor(ui_color(m_widget, QPalette::Text));
  pen.setWidth(2);
  pen.setCosmetic(true);
  painter.setPen(pen);
  painter.drawRect(grid);
  painter.restore();

  for (const Anchor anchor : PROPER_ANCHORS) {
    draw_anchor(painter, grid, anchor);
  }
}

bool AnchorHUD::mouse_press(QMouseEvent& event)
{
  const QPointF pos = event.pos() - this->pos;
  const QRectF grid = anchor_grid();
  for (const Anchor anchor : PROPER_ANCHORS) {
    if (anchor_rect(Options::anchor_position(grid, anchor)).contains(pos)) {
      if (m_anchor == anchor) {
        set_anchor(Anchor::None);
      } else {
        set_anchor(anchor);
      }
      m_disable_hover_for = anchor;
      QTimer::singleShot(1000, [this]() { m_disable_hover_for = Anchor::None; });
      return true;
    }
  }
  return false;
}

void AnchorHUD::mouse_release(QMouseEvent& event)
{
  Q_UNUSED(event);
}

bool AnchorHUD::mouse_move(QMouseEvent& event)
{
  m_mouse_pos = event.pos() - this->pos;
  m_disable_hover = false;
  return false;
}

void AnchorHUD::set_anchor(const AnchorHUD::Anchor& anchor)
{
  if (m_anchor != anchor) {
    m_anchor = anchor;
    Q_EMIT anchor_changed(anchor);
  }
}

QRectF AnchorHUD::anchor_grid() const
{
  return QRectF(QPointF(ANCHOR_RADIUS, ANCHOR_RADIUS),
                size() - QSizeF(2*ANCHOR_RADIUS, 2*ANCHOR_RADIUS));
}

void AnchorHUD::draw_anchor(QPainter& painter, const QRectF& grid, Anchor anchor) const
{
  const QPointF pos = Options::anchor_position(grid, anchor);
  const bool is_selected = m_anchor == anchor;
  painter.save();
  if (std::isnan(pos.x()) || std::isnan(pos.y())) {
    return;
  }
  QRectF rect = anchor_rect(pos);
  QPen pen;
  pen.setColor(ui_color(m_widget, QPalette::Text));
  painter.setPen(pen);
  QBrush brush(Qt::SolidPattern);
  if (rect.contains(m_mouse_pos) && m_disable_hover_for != anchor && !m_disable_hover) {
    brush.setColor(ui_color(m_widget, "AnchorHUD", "hover"));
  } else if (is_selected) {
    brush.setColor(ui_color(m_widget, "AnchorHUD", "checked"));
  } else {
    brush.setColor(ui_color(m_widget, QPalette::Window));
  }
  painter.setBrush(brush);
  painter.drawEllipse(rect);
  painter.restore();
}

}  // namespace omm
