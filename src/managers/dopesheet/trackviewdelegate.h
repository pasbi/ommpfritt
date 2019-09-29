#include <QAbstractItemDelegate>
#include <set>
#include "managers/timeline/timelinecanvas.h"

namespace omm
{

class Animator;
class Track;
class DopeSheetView;
class Property;


class TrackViewDelegate : public QAbstractItemDelegate
{
  Q_OBJECT
public:
  TrackViewDelegate(DopeSheetView& view, Animator& animator);
  void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
  QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex &index) const override;

  struct TrackGeometry
  {
    explicit TrackGeometry(const TrackViewDelegate& delegate,
                           const QRect& item_rect);
    const QRectF rect;
  };

  void mouse_move(QMouseEvent& event);
  bool mouse_button_press(QMouseEvent& event);
  void mouse_button_release(QMouseEvent&event);
  void abort_move();

public Q_SLOTS:
  void remove_keyframe(Track& track, int frame);
  void insert_keyframe(Track& track, int frame);
  void move_keyframe(Track& track, int old_frame, int new_frame);
  void remove_track(Track& track);
  void insert_track(Track& track);
  void invalidate_cache();
  void deselect_all_keyframes_except(Track* e_track, int frame);

private:
  DopeSheetView& m_view;
  Animator& m_animator;

  struct PersistentKeyFrame
  {
    bool is_selected = false;
    bool is_dragged = false;
  };

  std::map<Track*, std::map<int, PersistentKeyFrame>> m_keyframes;

  enum class KeyFrameStyle { Normal, Ghost };
  void draw_keyframe(QPainter& painter, const QRectF& rect, const PersistentKeyFrame& k,
                     KeyFrameStyle style) const;
  void draw_background(QPainter& painter, const QRect& rect) const;
  void draw_current(QPainter& painter, const QRect& rect) const;
  QPoint map_to_track_view(const QModelIndex& index, const QPoint& point) const;
  TrackGeometry calculate_geometry() const;

  double frame_to_pixel(double frame) const;
  int pixel_to_frame(int pixel) const;
  double pixel_per_frame(int width) const;
  int left() const;
  int width() const;
  QPoint m_mouse_down_pos;
  bool m_move_aborted = false;
  int m_current_shift = 0;

  class TimelineCanvasC : public TimelineCanvas
  {
  public:
    TimelineCanvasC(Animator& animator, TrackViewDelegate& self);
    void update() override;
    QPoint map_to_global(const QPoint &pos) const override;
  private:
    TrackViewDelegate& m_self;
  };

  mutable TimelineCanvasC m_canvas;
};

}  // namespace omm
