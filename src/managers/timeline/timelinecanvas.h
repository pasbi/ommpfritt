#pragma once

#include <QRectF>
#include <set>
#include <QPainter>
#include <QFont>
#include <QObject>

#include <animation/animator.h>

class QMouseEvent;
class QKeyEvent;

namespace omm
{

class Animator;
class Track;

class TimelineCanvas : public QObject
{
  Q_OBJECT
public:
  explicit TimelineCanvas(Animator& animator);

  void draw_background(QPainter& painter) const;
  void draw_lines(QPainter& painter) const;
  void draw_keyframes(QPainter& painter) const;
  void draw_current(QPainter& painter) const;
  void draw_fcurve(QPainter& painter) const;

  double ppf() const;
  double ppfs() const;

  double left_frame = 1;
  double right_frame = 100;
  QRectF rect;
  std::set<Track*> tracks;
  Animator& animator;
  int footer_height = 0;

  bool view_event(QEvent& event);

  virtual QPoint map_to_global(const QPoint& pos) const = 0;
  virtual void update() = 0;

Q_SIGNALS:
  void current_frame_changed(int);

private:
  std::map<Track*, std::set<int>> m_selection;
  double pixel_to_frame(double pixel) const;
  double frame_to_pixel(double frame) const;
  double normalized_to_frame(double pixel) const;
  double frame_to_normalized(double frame) const;
  double footer_y() const;
  std::set<Track*> tracks_at(double frame) const;
  bool is_selected(int frame) const;
  void select(int frame);

  bool m_pan_active = false;
  bool m_zoom_active = false;
  bool m_dragging_knots = false;
  bool m_dragging_time = false;
  bool m_move_aborted = false;
  int m_shift = 0;
  QPoint m_last_mouse_pos;
  QPoint m_mouse_down_pos;

  bool mouse_press(QMouseEvent& event);
  bool mouse_move(QMouseEvent& event);
  bool mouse_release(QMouseEvent& event);
  bool key_press(QKeyEvent& event);
};

}  // namespace omm
