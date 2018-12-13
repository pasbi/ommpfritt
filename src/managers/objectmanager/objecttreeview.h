#pragma once

#include <QTreeView>
#include "common.h"
#include "managers/manageritemview.h"
#include "managers/objectmanager/tagsitemdelegate.h"
#include "scene/objecttreeadapter.h"

namespace omm
{

class Object;

class ObjectTreeView : public ManagerItemView<QTreeView, ObjectTreeAdapter>
{
public:
  using model_type = ObjectTreeAdapter;
  explicit ObjectTreeView(ObjectTreeAdapter& model);
  void remove_selected_tags(Object& object) const;

protected:
  void populate_menu(QMenu& menu, const QModelIndex& index) const override;

private:
  std::unique_ptr<TagsItemDelegate> m_tags_item_delegate;
};

}  // namespace
