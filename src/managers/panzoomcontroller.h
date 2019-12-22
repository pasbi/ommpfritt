#pragma once

#include <QTransform>
#include <QPoint>

namespace omm
{

class PanZoomController
{
public:
  PanZoomController(QWidget& widget);
  enum class Action { Pan, Zoom, None };
  void start(Action action);
  void end();
  bool move(QPointF pos);

  QTransform transform() const { return m_transform; }
  QPointF last_mouse_pos() const { return m_last_pos; }
  QPointF pixel_d() const { return m_pixel_d; }
  QPointF unit_d() const { return m_unit_d; }
  static void draw_rubberband(QPainter& painter, const QWidget& widget, const QRectF& rect);
  void draw_rubberband(QPainter& painter) const;
  bool rubber_band_visible = false;
  QPointF offset() const;
  QRectF unit_rubber_band() const;
  void translate(const QPointF& t);

private:
  Action m_current_action = Action::None;
  QPointF m_last_pos;
  QPointF m_start_pos;
  QPointF m_pixel_d;
  QPointF m_unit_d;
  void zoom();

  QTransform m_transform;
  QWidget& m_widget;
};

}  // namespace omm
