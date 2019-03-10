#pragma once

#include <QAbstractItemDelegate>

namespace omm
{

class Object;
class ObjectTreeView;
class Tag;
class ObjectTreeSelectionModel;

class ObjectQuickAccessDelegate : public QAbstractItemDelegate
{
public:
  explicit ObjectQuickAccessDelegate(ObjectTreeView& view);
  void paint( QPainter *painter, const QStyleOptionViewItem &option,
              const QModelIndex &index ) const override;
  QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
  bool editorEvent( QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                    const QModelIndex &index ) override;

  static constexpr int width = 40;
  static constexpr QRectF enabled_cross_area = QRectF(QPointF(0.0, 0.0), QSizeF(0.5, 1.0));
  static constexpr QRectF edit_visibility = QRectF(QPointF(0.5, 0.0), QSizeF(0.5, 0.5));
  static constexpr QRectF export_visibility = QRectF(QPointF(0.5, 0.5), QSizeF(0.5, 0.5));

private:
  ObjectTreeView& m_view;
  bool on_mouse_button_press(QMouseEvent& event);
};

}  // namespace omm
