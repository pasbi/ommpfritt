#pragma once

#include <QTreeView>
#include "common.h"
#include "managers/manageritemview.h"
#include "managers/objectmanager/tagsitemdelegate.h"
#include "managers/objectmanager/objecttreeadapter.h"

namespace omm
{

class Object;

class ObjectTreeView : public ManagerItemView<QTreeView, ObjectTreeAdapter>
{
public:
  explicit ObjectTreeView(ObjectTreeAdapter& model);
  void set_selection(const std::set<AbstractPropertyOwner*>& selection) override;
  AbstractPropertyOwner::Kind displayed_kinds() const override;
  void remove_selected_tags(Object& object) const;

protected:
  void populate_menu(QMenu& menu, const QModelIndex& index) const override;

private:
  std::unique_ptr<TagsItemDelegate> m_tags_item_delegate;
  void set_selection(const std::set<AbstractPropertyOwner*>& selection, Object& root);

};

}  // namespace
