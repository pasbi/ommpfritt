#pragma once

#include <QRectF>
#include <set>
#include <QPainter>
#include <QFont>
#include <QObject>

class QMouseEvent;

namespace omm
{

class Animator;
class Track;

class TimelineCanvas : public QObject
{
  Q_OBJECT
public:
  explicit TimelineCanvas(Animator& animator);

  void set_font(const QFont& font);

  void draw(QPainter& painter) const;
  void draw_background(QPainter& painter) const;
  void draw_lines(QPainter& painter) const;
  void draw_keyframes(QPainter& painter) const;
  void draw_current(QPainter& painter) const;

  double ppf() const;
  double ppfs() const;

  double left_frame = 1;
  double right_frame = 100;
  QRectF rect;
  std::set<Track*> tracks;

  void mouse_press(QMouseEvent& event);
  void mouse_move(QMouseEvent& event);
  void mouse_release(QMouseEvent& event);

  virtual QPoint map_to_global(const QPoint& pos) const = 0;
  virtual void update() = 0;

Q_SIGNALS:
  void current_frame_changed(int);

private:
  const Animator& m_animator;
  QFont m_font;
  bool m_draw_text = false;
  std::map<Track*, std::set<int>> m_selection;
  double pixel_to_frame(double pixel) const;
  double frame_to_pixel(double pixel) const;

  bool m_pan_active;
  bool m_zoom_active;
  QPoint m_last_mouse_pos;
  QPoint m_mouse_down_pos;
};

}  // namespace omm
