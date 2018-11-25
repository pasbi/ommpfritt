#pragma once

#include <QAbstractItemDelegate>

namespace omm
{

class Object;
class ObjectTreeView;

class TagsItemDelegate : public QAbstractItemDelegate
{
public:
  explicit TagsItemDelegate(ObjectTreeView& view);
  void set_object(Object& object);

  void paint( QPainter *painter, const QStyleOptionViewItem &option,
              const QModelIndex &index ) const override;
  QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
  bool editorEvent( QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                    const QModelIndex &index ) override;

private:
  ObjectTreeView& m_view;
  QPoint cell_pos(const QModelIndex& index) const;
};

}  // namespace omm