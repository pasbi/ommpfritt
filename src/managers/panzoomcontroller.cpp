#include "managers/panzoomcontroller.h"
#include <QWidget>
#include "logging.h"
#include <QPainter>
#include "preferences/uicolors.h"
#include <cmath>

namespace omm
{

PanZoomController::PanZoomController(QWidget& widget) : m_widget(widget)
{

}

void PanZoomController::start(PanZoomController::Action action)
{
  m_current_action = action;
  m_start_pos = m_last_pos;
}

void PanZoomController::end()
{
  m_current_action = Action::None;
}

bool PanZoomController::move(QPointF pos)
{
  pos -= offset();
  m_pixel_d = pos - m_last_pos;
  bool invertible;
  const QTransform ti = m_transform.inverted(&invertible);
  assert(invertible);
  m_unit_d = ti.map(pos) - ti.map(m_last_pos);
  m_last_pos = pos;
  switch (m_current_action) {
  case Action::Pan:
    pan();
    return true;
  case Action::Zoom:
    zoom();
    return true;
  default:
    return false;
  }
}

void PanZoomController::draw_rubberband(QPainter& painter) const
{
  if (rubber_band_visible) {
    const QPointF o = offset();
    const auto rect = QRectF(m_start_pos+o, m_last_pos+o).normalized();
    draw_rubberband(painter, m_widget, rect);
  }
}

QPointF PanZoomController::offset() const
{
  return QPointF(m_widget.width(), m_widget.height())/2.0;
}

QRectF PanZoomController::unit_rubber_band() const
{
  bool i;
  const QTransform ti = m_transform.inverted(&i);
  assert(i);
  return QRectF(ti.map(m_last_pos), ti.map(m_start_pos)).normalized();
}

void PanZoomController::draw_rubberband(QPainter& painter, const QWidget& widget, const QRectF& rect)
{
  LINFO << "draw rubberband: " << rect;
  painter.save();
  painter.resetTransform();
  QPen pen;
  pen.setWidth(2.0);
  pen.setCosmetic(true);
  pen.setColor(ui_color(widget, "TimeLine", "rubberband outline"));
  painter.setPen(pen);
  painter.fillRect(rect, ui_color(widget, "TimeLine", "rubberband fill"));
  painter.drawRect(rect);
  painter.restore();
}

void PanZoomController::pan()
{
  QTransform t;
  t.translate(m_pixel_d.x(), m_pixel_d.y());
  m_transform  = m_transform * t;
}

void PanZoomController::zoom()
{
  const double sign = std::abs(m_pixel_d.x()) > std::abs(m_pixel_d.y()) ? m_pixel_d.x() : m_pixel_d.y();
  const double d2 = std::copysign(std::sqrt(QPointF::dotProduct(m_unit_d, m_unit_d)), sign);
  const double s = std::exp(d2 / 600.0);

  QTransform tt;
  tt.translate(-m_start_pos.x(), -m_start_pos.y());

  QTransform ts;
  const double det = m_transform.determinant();
  assert(det > 0);
  static constexpr auto lower = 0.01;
  static constexpr auto upper = 100.0;
  if ((det >= lower || s > 1.0) && (det <= upper || s < 1.0)) {
    ts.scale(s, s);
  }

  QTransform tti;
  tti.translate(m_start_pos.x(), m_start_pos.y());

  m_transform = m_transform * tt * ts * tti;
}

}  // namespace
