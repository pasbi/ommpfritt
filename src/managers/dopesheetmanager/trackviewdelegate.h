#pragma once

#include <QAbstractItemDelegate>
#include <set>

namespace omm
{
class Track;
class DopeSheetView;
class Property;
class TimelineCanvas;

class TrackViewDelegate : public QAbstractItemDelegate
{
  Q_OBJECT
public:
  TrackViewDelegate(DopeSheetView& view, TimelineCanvas& canvas);
  void paint(QPainter* painter,
             const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;
  [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem&,
                               const QModelIndex& index) const override;

  bool view_event(QEvent& event);
  void activate_index(const QModelIndex& index) const;

private:
  DopeSheetView& m_view;
  TimelineCanvas& m_canvas;

  QModelIndex m_mouse_press_index;

  [[nodiscard]] bool display_fcurve(const QModelIndex& index) const;
};

}  // namespace omm
