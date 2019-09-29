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

  void mouse_move(QMouseEvent& event);
  void mouse_press(QMouseEvent& event);
  void mouse_release(QMouseEvent& event);
  void key_press(QKeyEvent& event);

  void toggle_expanded(const QModelIndex& index);

private:
  DopeSheetView& m_view;
  Animator& m_animator;

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
  void activate_index(const QModelIndex& index) const;
  QModelIndex m_mouse_press_index;
  std::set<Track*> m_expanded_tracks;
  bool display_fcurve(const QModelIndex& index) const;
};

}  // namespace omm
