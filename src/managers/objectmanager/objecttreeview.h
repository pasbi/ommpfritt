#pragma once

#include <QTreeView>
#include "common.h"
#include "managers/manageritemview.h"
#include "managers/objectmanager/tagsitemdelegate.h"
#include "scene/objecttreeadapter.h"
#include "managers/objectmanager/objecttreeselectionmodel.h"

namespace omm
{

class Object;

class ObjectTreeView : public ManagerItemView<QTreeView, ObjectTreeAdapter>
{
public:
  using model_type = ObjectTreeAdapter;
  explicit ObjectTreeView(ObjectTreeAdapter& model);
  std::set<AbstractPropertyOwner*> selected_items() const override;
  std::set<AbstractPropertyOwner*> selected_objects() const;
  std::set<AbstractPropertyOwner*> selected_tags() const;

protected:
  std::vector<std::string> application_actions() const override;
  void paintEvent(QPaintEvent* e) override;
  void dragEnterEvent(QDragEnterEvent* e) override;
  void dragMoveEvent(QDragMoveEvent* e) override;
  void mouseMoveEvent(QMouseEvent* e) override;
  void mousePressEvent(QMouseEvent* e) override;

private:
  std::unique_ptr<ObjectTreeSelectionModel> m_selection_model;
  std::unique_ptr<TagsItemDelegate> m_tags_item_delegate;
  ObjectTreeAdapter& m_model;

  QPoint m_mouse_press_pos;
  QModelIndex m_dragged_index;

  void handle_drag_event(QDragMoveEvent* e);
};

}  // namespace
