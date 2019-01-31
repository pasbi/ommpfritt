#pragma once

#include <QAbstractItemDelegate>

namespace omm
{

class Object;
class ObjectTreeView;
class Tag;
class ObjectTreeSelectionModel;

class TagsItemDelegate : public QAbstractItemDelegate
{
public:
  explicit TagsItemDelegate(ObjectTreeView& view, ObjectTreeSelectionModel& selection_model);
  void paint( QPainter *painter, const QStyleOptionViewItem &option,
              const QModelIndex &index ) const override;
  QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
  bool editorEvent( QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                    const QModelIndex &index ) override;
  Tag* tag_at(const QModelIndex& index, const QPoint& pos) const;
  Tag* tag_at(const QPoint& pos) const;
  Tag* tag_before(const QModelIndex& index, QPoint pos) const;
  Tag* tag_before(const QPoint& pos) const;

private:
  ObjectTreeView& m_view;
  ObjectTreeSelectionModel& m_selection_model;
  QPoint cell_pos(const QModelIndex& index) const;

  bool on_mouse_button_press(QMouseEvent& event);
  bool on_mouse_button_release(QMouseEvent& event);
  bool m_fragile_selection = false;
};

}  // namespace omm
