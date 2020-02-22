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
#include "managers/nodemanager/nodeview.h"

namespace omm
{

PanZoomController::PanZoomController(NodeView& view) : m_view(view)
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
    m_view.reset_scene_rect();
    QPointF current_scene_pos = m_view.mapToScene(m_start_widget_pos);
    const QPointF d = m_start_scene_pos - current_scene_pos;
    m_view.setSceneRect(m_view.sceneRect().translated(d));
  }
    break;
  case Action::Pan:
    scene_rect.translate(map_direction_to_scene(-d));
    m_view.setSceneRect(scene_rect);
    break;
  default:
    return false;
  }
  return true;
}

void PanZoomController::release()
{
  m_current_action = Action::None;
}

}  // namespace
