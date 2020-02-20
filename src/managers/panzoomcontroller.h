#pragma once

#include <QTransform>
#include <QPoint>

class QMouseEvent;
class QGraphicsView;

namespace omm
{

class PanZoomController
{
public:
  PanZoomController(QGraphicsView& view);
  enum class Action { Pan, Zoom, None };
  bool press(const QMouseEvent& event);
  bool move(const QMouseEvent& event);
  void release();

  QTransform transform() const { return m_transform; }
  QPointF last_mouse_pos() const { return m_last_pos; }
  void translate(const QPointF& t);

private:
  Action m_current_action = Action::None;
  QPoint m_last_pos;
  QPoint m_start_widget_pos;
  QPointF m_start_scene_pos;

  QTransform m_transform;
  QGraphicsView& m_view;
};

}  // namespace omm
