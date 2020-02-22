#include "managers/panzoomcontroller.h"
#include <QMouseEvent>
#include "preferences/preferences.h"
#include <QWidget>
#include "logging.h"
#include <QPainter>
#include "preferences/uicolors.h"
#include <cmath>
#include "mainwindow/application.h"
#include <QGraphicsView>

namespace
{

QPointF scale(const QPointF& p, double f, const QPointF& origin)
{
  return (p - origin) * f + origin;
}

QRectF scale(const QRectF& r, double f, const QPointF& origin)
{
  return QRectF(scale(r.topLeft(), f, origin),
                scale(r.bottomRight(), f, origin));
}

QRectF mapAABB(const QRectF& r, const QTransform& t)
{
  return QRectF(t.map(r.topLeft()), t.map(r.bottomRight()));
}

}

namespace omm
{

PanZoomController::PanZoomController(QGraphicsView& view) : m_view(view)
{

}

void PanZoomController::translate(const QPointF& v)
{
  QTransform t;
  t.translate(v.x(), v.y());
  m_transform = m_transform * t;
}

bool PanZoomController::press(const QMouseEvent& event)
{
  m_start_widget_pos = event.pos();
  m_start_scene_pos = m_view.mapToScene(m_start_widget_pos);
  m_last_pos = event.pos();
  const auto& preferences = Application::instance().preferences;
  if (preferences.match("shift viewport", event, true)) {
    m_current_action = Action::Pan;
    return true;
  } else if (preferences.match("zoom viewport", event, true)) {
    m_current_action = Action::Zoom;
    return true;
  } else {
    m_current_action = Action::None;
    return false;
  }
}

bool PanZoomController::move(const QMouseEvent& event)
{
  const auto map_direction_to_scene = [this](const QPoint& direction) {
    return m_view.mapToScene(direction) - m_view.mapToScene(QPoint());
  };

  QPoint current_pos = event.pos();
  const QPoint d = current_pos - m_last_pos;
  m_last_pos = current_pos;
  QRectF scene_rect = m_view.sceneRect();
  switch (m_current_action) {
  case Action::Zoom:
  {
    double f = std::pow(1.005, d.x());
    const double current_scale = m_view.transform().determinant();
    if (current_scale > max_scale) {
      // only allow zoom out
      f = std::min(1.0, f);
    } else if (current_scale < min_scale) {
      // only allow zoom in
      f = std::max(1.0, f);
    }
    m_view.scale(f, f);
    scene_rect = mapAABB(m_view.rect(), m_view.transform().inverted());
    m_view.setSceneRect(scene_rect);
    QPointF current_scene_pos = m_view.mapToScene(m_start_widget_pos);
    scene_rect.translate(m_start_scene_pos - current_scene_pos);
    m_view.setSceneRect(scene_rect);
  }
    break;
  case Action::Pan:
    scene_rect.translate(map_direction_to_scene(-d));
    m_view.setSceneRect(scene_rect);
    break;
  default:
    return false;
  }
  LINFO << scene_rect;
  return true;
}

void PanZoomController::release()
{
  m_current_action = Action::None;
}

//void PanZoomController::zoom()
//{
//  const double sign = std::abs(m_pixel_d.x()) > std::abs(m_pixel_d.y()) ? m_pixel_d.x() : m_pixel_d.y();
//  const double d2 = std::copysign(std::sqrt(QPointF::dotProduct(m_unit_d, m_unit_d)), sign);
//  const double s = std::exp(d2 / 600.0);

//  QTransform tt;
//  tt.translate(-m_start_pos.x(), -m_start_pos.y());

//  QTransform ts;
//  const double det = m_transform.determinant();
//  assert(det > 0);
//  static constexpr auto lower = 0.01;
//  static constexpr auto upper = 1.0;
//  if ((det >= lower || s > 1.0) && (det <= upper || s < 1.0)) {
//    ts.scale(s, s);
//  }

//  QTransform tti;
//  tti.translate(m_start_pos.x(), m_start_pos.y());

//  m_transform = m_transform * tt * ts * tti;
//}

}  // namespace
