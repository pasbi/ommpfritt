#pragma once

#include <QAbstractItemDelegate>
#include "scene/history/macro.h"
#include <memory>

namespace omm
{

class Object;
class ObjectTreeView;
class Tag;
class ObjectTreeSelectionModel;

class ObjectQuickAccessDelegate : public QAbstractItemDelegate
{
public:
  enum class ActiveItem { None, Activeness, Visibility };
  explicit ObjectQuickAccessDelegate(ObjectTreeView& view);
  void paint( QPainter *painter, const QStyleOptionViewItem &option,
              const QModelIndex &index ) const override;
  QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  static constexpr int width = 40;
  static constexpr QRectF enabled_cross_area = QRectF(QPointF(0.0, 0.0), QSizeF(0.5, 1.0));
  static constexpr QRectF edit_visibility = QRectF(QPointF(0.5, 0.0), QSizeF(0.5, 0.5));
  static constexpr QRectF export_visibility = QRectF(QPointF(0.5, 0.5), QSizeF(0.5, 0.5));
  bool on_mouse_button_press(QMouseEvent& event);
  void on_mouse_move(QMouseEvent& event);
  void on_mouse_release(QMouseEvent& event);
  QPointF to_local(const QPoint& view_global) const;

private:
  ObjectTreeView& m_view;

  ActiveItem m_active_item = ActiveItem::None;
  int m_active_item_value;
  std::unique_ptr<Macro> m_macro;
};

}  // namespace omm
