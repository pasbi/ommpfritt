#pragma once

#include <QTransform>
#include <QPoint>

namespace omm
{

class PanZoomController
{
public:
  enum class Action { Pan, Zoom, None };
  void start(Action action);
  void end();
  bool move(const QPoint& pos);

  QTransform transform() const { return m_transform; }
  QPoint last_mouse_pos() const { return m_last_pos; }
  QPoint d() const { return m_d; }
  QPointF e() const { return m_e; }

private:
  Action m_current_action = Action::None;
  QPoint m_last_pos;
  QPoint m_start_pos;
  QPoint m_d;
  QPointF m_e;
  void pan();
  void zoom();

  QTransform m_transform;
};

}  // namespace omm
