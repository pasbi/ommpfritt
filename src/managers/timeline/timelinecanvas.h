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
  explicit TimelineCanvas(Animator& animator, QWidget& widget);

  void draw_background(QPainter& painter) const;
  void draw_lines(QPainter& painter) const;
  void draw_keyframes(QPainter& painter) const;
  void draw_current(QPainter& painter) const;
  void draw_rubber_band(QPainter& painter) const;

  std::set<Track*> tracks;
  Animator& animator;
  int footer_height = 0;
  QRectF rect;

  bool view_event(QEvent& event);

  virtual QPoint map_to_global(const QPoint& pos) const = 0;

  /**
   * @brief disable_context_menu disable the context menu (right click) in the view
   */
  virtual void disable_context_menu() = 0;

  /**
   * @brief enable_context_menu enables the context menu (right click) in the view
   */
  virtual void enable_context_menu() = 0;

  /**
   * @brief track_rect returns the geometry of the area represented by the given track.
   *  If @code track is not visible in the view, an empty rect is returned.
   */
  virtual QRect track_rect(Track& track) = 0;

  /**
   * @brief owner_rect returs the geometry of the area represented by the given owner.
   *  If @code owner is not visible in the view or the view does not displays owners at all, an
   *  empty rect is returned.
   */
  virtual QRect owner_rect(AbstractPropertyOwner& owner) = 0;

public Q_SLOTS:
  /**
   * @brief update issues a redraw of the gui
   */
  virtual void update() = 0;

Q_SIGNALS:
  void current_frame_changed(int);

public:
  struct Range
  {
    Range(double begin, double end) : begin(begin), end(end) {}
    double begin;
    double end;
    virtual int pixel_range() const = 0;
    double units_per_pixel() const;
    double pixel_to_unit(double pixel) const;
    double unit_to_pixel(double unit) const;
    double unit_to_normalized(double unit) const;
    double normalized_to_unit(double normalized) const;
    double width() const;
    void pan(double d);
    void zoom(double origin, double amount, double min_upp, double max_upp);
  };

  struct PixelRange : Range {
    PixelRange(TimelineCanvas& self) : Range(1, 100), m_self(self) {}
    int pixel_range() const override { return m_self.rect.width(); }
  private:
    TimelineCanvas& m_self;
  } frame_range;

protected:
  const QWidget& m_widget;
private:
  std::map<Track*, std::set<int>> m_selection;
  std::map<Track*, std::set<int>> m_rubber_band_selection;

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

  bool mouse_press(QMouseEvent& event);
  bool mouse_move(QMouseEvent& event);
  bool mouse_release(QMouseEvent& event);
  bool key_press(QKeyEvent& event);
  QPoint m_rubber_band_origin;
  QPoint m_rubber_band_corner;
  bool m_rubber_band_visible = false;
  QRect rubber_band() const;

protected:
  QPoint m_mouse_down_pos;
  virtual void pan(const QPointF& d);
  virtual void zoom(const QPointF& d);
  void synchronize_track_selection_with_animator();
  void update_tracks(const std::set<AbstractPropertyOwner*>& selection);
};

}  // namespace omm
