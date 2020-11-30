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
                            int column);
  void paint(QPainter* painter,
             const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;
  [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem&,
                               const QModelIndex& index) const override;
  bool editorEvent(QEvent* event,
                   QAbstractItemModel*,
                   const QStyleOptionViewItem&,
                   const QModelIndex&) override;
  [[nodiscard]] Tag* tag_at(const QModelIndex& index, const QPoint& pos) const;
  [[nodiscard]] Tag* tag_at(const QPoint& pos) const;
  [[nodiscard]] Tag* tag_before(const QModelIndex& index, QPoint pos) const;
  [[nodiscard]] Tag* tag_before(const QPoint& pos) const;
  [[nodiscard]] static QSize tag_icon_size();
  [[nodiscard]] static int advance();
  [[nodiscard]] std::set<Tag*> tags(const QModelIndex& index, const QRect& rect) const;
  [[nodiscard]] static QRect tag_rect(const QPoint& base, std::size_t i);

  QRect rubberband;
  QItemSelectionModel::SelectionFlag selection_flag = QItemSelectionModel::Select;

private:
  ObjectTreeView& m_view;
  ObjectTreeSelectionModel& m_selection_model;
  [[nodiscard]] QPoint cell_pos(const QModelIndex& index) const;

  bool on_mouse_button_press(QMouseEvent& event);
  bool on_mouse_button_release(QMouseEvent& event);
  bool m_fragile_selection = false;
  const int m_column;
};

}  // namespace omm
