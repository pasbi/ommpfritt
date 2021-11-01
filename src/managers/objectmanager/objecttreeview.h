#pragma once

#include "common.h"
#include "managers/manageritemview.h"
#include "scene/objecttree.h"
#include <QTreeView>

namespace omm
{
class AbstractPropertyOwner;
class ObjectDelegate;
class Object;
class ObjectTreeSelectionModel;
class ObjectQuickAccessDelegate;
class TagsItemDelegate;

class ObjectTreeView : public ManagerItemView<QTreeView, ObjectTree>
{
  Q_OBJECT
public:
  using model_type = ObjectTree;
  explicit ObjectTreeView(ObjectTree& model);
  ~ObjectTreeView() override;
  [[nodiscard]] std::set<AbstractPropertyOwner*> selected_items() const override;
  [[nodiscard]] std::set<AbstractPropertyOwner*> selected_objects() const;
  [[nodiscard]] std::set<AbstractPropertyOwner*> selected_tags() const;

  [[nodiscard]] Scene& scene() const;
  static constexpr int row_height = 20;

public:
  void set_selection(const std::set<AbstractPropertyOwner*>& selected_items);
  void update_tag_column_size();

protected:
  void dragEnterEvent(QDragEnterEvent* e) override;
  void dragMoveEvent(QDragMoveEvent* e) override;
  void mouseMoveEvent(QMouseEvent* e) override;
  void mousePressEvent(QMouseEvent* e) override;
  void mouseReleaseEvent(QMouseEvent* e) override;
  void focusInEvent(QFocusEvent* e) override;
  void paintEvent(QPaintEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;

private:
  std::unique_ptr<ObjectTreeSelectionModel> m_selection_model;
  std::unique_ptr<ObjectQuickAccessDelegate> m_object_quick_access_delegate;
  std::unique_ptr<TagsItemDelegate> m_tags_item_delegate;
  std::unique_ptr<QTimer> m_update_timer;
  ObjectTree& m_model;

  QPoint m_mouse_press_pos;
  QModelIndex m_mouse_down_index;

  void handle_drag_event(QDragMoveEvent* e);

  QPoint m_rubberband_origin;
  QPoint m_rubberband_corner;
  [[nodiscard]] QRect rubber_band() const;
  bool m_rubberband_visible = false;
  [[nodiscard]] QModelIndexList indices(QRect rect) const;
  ObjectDelegate* m_object_delegate;
  bool m_aborted = false;
};

}  // namespace omm
