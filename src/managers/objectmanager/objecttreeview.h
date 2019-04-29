#pragma once

#include <QTreeView>
#include "common.h"
#include "managers/manageritemview.h"
#include "managers/objectmanager/tagsitemdelegate.h"
#include "managers/objectmanager/objectquickaccessdelegate.h"
#include "scene/objecttreeadapter.h"
#include "managers/objectmanager/objecttreeselectionmodel.h"

namespace omm
{

class Object;

class ObjectTreeView : public ManagerItemView<QTreeView, ObjectTreeAdapter>
{
  Q_OBJECT
public:
  using model_type = ObjectTreeAdapter;
  explicit ObjectTreeView(ObjectTreeAdapter& model);
  std::set<AbstractPropertyOwner*> selected_items() const override;
  std::set<AbstractPropertyOwner*> selected_objects() const;
  std::set<AbstractPropertyOwner*> selected_tags() const;

  Scene& scene() const;

public Q_SLOTS:
  void set_selection(const std::set<AbstractPropertyOwner*>& selected_items);

protected:
  void paintEvent(QPaintEvent* e) override;
  void dragEnterEvent(QDragEnterEvent* e) override;
  void dragMoveEvent(QDragMoveEvent* e) override;
  void mouseMoveEvent(QMouseEvent* e) override;
  void mousePressEvent(QMouseEvent* e) override;
  void mouseReleaseEvent(QMouseEvent *e) override;
  void focusInEvent(QFocusEvent *e) override;

private:
  std::unique_ptr<ObjectTreeSelectionModel> m_selection_model;
  std::unique_ptr<ObjectQuickAccessDelegate> m_object_quick_access_delegate;
  std::unique_ptr<TagsItemDelegate> m_tags_item_delegate;
  std::unique_ptr<QTimer> m_update_timer;
  ObjectTreeAdapter& m_model;

  QPoint m_mouse_press_pos;
  QModelIndex m_dragged_index;

  void handle_drag_event(QDragMoveEvent* e);
};

}  // namespace
