#pragma once

#include <QAbstractItemDelegate>
#include <QItemSelectionModel>
#include <set>

namespace omm
{
class Object;
class ObjectTreeView;
class Tag;
class ObjectTreeSelectionModel;

class TagsItemDelegate : public QAbstractItemDelegate
{
public:
  explicit TagsItemDelegate(ObjectTreeView& view,
                            ObjectTreeSelectionModel& selection_model,
                            const int column);
  void paint(QPainter* painter,
             const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;
  QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex& index) const override;
  bool editorEvent(QEvent* event,
                   QAbstractItemModel*,
                   const QStyleOptionViewItem&,
                   const QModelIndex&) override;
  Tag* tag_at(const QModelIndex& index, const QPoint& pos) const;
  Tag* tag_at(const QPoint& pos) const;
  Tag* tag_before(const QModelIndex& index, QPoint pos) const;
  Tag* tag_before(const QPoint& pos) const;
  QSize tag_icon_size() const;
  int advance() const;
  std::set<Tag*> tags(const QModelIndex& index, const QRect& rect) const;
  QRect tag_rect(const QPoint& base, std::size_t i) const;

  QRect rubberband;
  QItemSelectionModel::SelectionFlag selection_flag = QItemSelectionModel::Select;

private:
  ObjectTreeView& m_view;
  ObjectTreeSelectionModel& m_selection_model;
  QPoint cell_pos(const QModelIndex& index) const;

  bool on_mouse_button_press(QMouseEvent& event);
  bool on_mouse_button_release(QMouseEvent& event);
  bool m_fragile_selection = false;
  const int m_column;
};

}  // namespace omm
