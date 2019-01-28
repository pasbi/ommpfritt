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
  bool remove_selection() override;

protected:
  void populate_menu(QMenu& menu, const QModelIndex& index) const override;
  void mousePressEvent(QMouseEvent* e) override;
  void mouseMoveEvent(QMouseEvent* e) override;

  constexpr static std::size_t TAGS_COLUMN = 2;
  constexpr static std::size_t VISIBILITY_COLUMN = 1;
  constexpr static std::size_t OBJECT_COLUMN = 0;

private:
  std::unique_ptr<ObjectTreeSelectionModel> m_selection_model;
  std::unique_ptr<TagsItemDelegate> m_tags_item_delegate;

  QPoint m_mouse_press_pos;
  QModelIndex m_dragged_index;
};

}  // namespace
