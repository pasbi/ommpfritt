#include <QAbstractItemDelegate>
#include <set>
#include "managers/timeline/timelinecanvas.h"

namespace omm
{

class Track;
class DopeSheetView;
class Property;

class TrackViewDelegate : public QAbstractItemDelegate
{
  Q_OBJECT
public:
  TrackViewDelegate(DopeSheetView& view, TimelineCanvas& canvas);
  void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
  QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex &index) const override;

  bool view_event(QEvent& event);
  void activate_index(const QModelIndex& index) const;

private:
  DopeSheetView& m_view;
  TimelineCanvas& m_canvas;

  QModelIndex m_mouse_press_index;

  bool display_fcurve(const QModelIndex& index) const;
};

}  // namespace omm
