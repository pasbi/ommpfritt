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

private:
  ObjectTreeView& m_view;
  ObjectTreeSelectionModel& m_selection_model;
  QPoint cell_pos(const QModelIndex& index) const;
};

}  // namespace omm