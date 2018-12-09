#include "managers/objectmanager/objecttreeview.h"

#include <memory>
#include <glog/logging.h>

#include "menuhelper.h"
#include "managers/objectmanager/objecttreeadapter.h"
#include "tags/tag.h"
#include "commands/attachtagcommand.h"
#include "renderers/style.h"

namespace omm
{

ObjectTreeView::ObjectTreeView(ObjectTreeAdapter& model)
  : ManagerItemView(model)
  , m_tags_item_delegate(std::make_unique<TagsItemDelegate>(*this))
{
  setItemDelegateForColumn(2, m_tags_item_delegate.get());
}

void ObjectTreeView::populate_menu(QMenu& menu, const QModelIndex& index) const
{
  auto* object = &model()->item_at(index);
  if (object->is_root()) {
    object = nullptr;
  }

  if (object != nullptr)
  {
    action(menu, tr("&remove"), *this, &ManagerItemView::remove_selection);
    auto tag_menu = std::make_unique<QMenu>(tr("&attach tag"));
    for (const auto& key : Tag::keys()) {
      action(*tag_menu, QString::fromStdString(key), [this, key](){
        attach_tag_to_selected(key);
      });
    }
    menu.addMenu(tag_menu.release());
  }
}

void ObjectTreeView::attach_tag_to_selected(const std::string& tag_class) const
{
  auto tag = Tag::make(tag_class);
  auto& scene = model()->scene();
  scene.submit<AttachTagCommand>(scene, std::move(tag));
}

void ObjectTreeView::set_selection(const SetOfPropertyOwner& selection, Object& root)
{
  assert(root.is_selected() == selection.count(static_cast<AbstractPropertyOwner*>(&root)));
  const QModelIndex index = model()->index_of(root);
  if (root.is_selected()) {
    selectionModel()->select(index, QItemSelectionModel::Rows | QItemSelectionModel::Select);
  } else {
    selectionModel()->select(index, QItemSelectionModel::Rows | QItemSelectionModel::Deselect);
  }
  for (auto child : root.children()) {
    set_selection(selection, *child);
  }
};

void ObjectTreeView::set_selection(const SetOfPropertyOwner& selection)
{
  set_selection(selection, model()->structure().root());
}

AbstractPropertyOwner::Kind ObjectTreeView::displayed_kinds() const
{
  return AbstractPropertyOwner::Kind::Object;
}

}  // namespace omm
