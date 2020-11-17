#pragma once

#include <QPoint>
#include <QTransform>

class QMouseEvent;

namespace omm
{
class NodeView;

class PanZoomController
{
public:
  PanZoomController(NodeView& view);
  enum class Action { Pan, Zoom, None };
  bool press(const QMouseEvent& event);
  bool move(const QMouseEvent& event);
  void release();

  QTransform transform() const
  {
    return m_transform;
  }
  QPointF last_mouse_pos() const
  {
    return m_last_pos;
  }
  double current_scale() const;

private:
  Action m_current_action = Action::None;
  QPoint m_last_pos;
  QPoint m_start_widget_pos;
  QPointF m_start_scene_pos;

  QTransform m_transform;
  NodeView& m_view;

  static constexpr double min_scale = 0.31;
  static constexpr double max_scale = 1.41;
};

}  // namespace omm
