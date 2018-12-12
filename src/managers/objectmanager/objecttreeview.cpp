#include "managers/objectmanager/objecttreeview.h"

#include <memory>
#include <glog/logging.h>

#include "menuhelper.h"
#include "commands/addtagcommand.h"
#include "scene/contextes.h"
#include "commands/removecommand.h"
#include "renderers/style.h"
#include "scene/scene.h"

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
    switch (index.column())
    {
    case 0:
    {
      action(menu, tr("&remove"), *this, &ManagerItemView::remove_selection);
      auto tag_menu = std::make_unique<QMenu>(tr("&attach tag"));
      for (const auto& key : Tag::keys()) {
        action(*tag_menu, QString::fromStdString(key), [this, key, object](){
          model()->scene().submit<AddTagCommand>(*object, Tag::make(key));
        });
      }
      menu.addMenu(tag_menu.release());
      break;
    }
    case 2:
      action(menu, tr("&remove"), [this, object]() {
        remove_selected_tags(*object);
      });
      break;
    }
  }
}

// void ObjectTreeView::set_selection(const SetOfPropertyOwner& selection, Object& root)
// {
//   // assert(root.is_selected() == selection.count(static_cast<AbstractPropertyOwner*>(&root)));
//   const QModelIndex index = model()->index_of(root);
//   if (root.is_selected()) {
//     LOG(INFO) << "select" << root;
//     selectionModel()->select(index, QItemSelectionModel::Rows | QItemSelectionModel::Select);
//   } else {
//     LOG(INFO) << "deselect" << root;
//     selectionModel()->select(index, QItemSelectionModel::Rows | QItemSelectionModel::Deselect);
//   }
//   for (auto child : root.children()) {
//     set_selection(selection, *child);
//   }
// };

// void ObjectTreeView::set_selection(const SetOfPropertyOwner& selection)
// {
//   set_selection(selection, model()->structure().root());
// }

AbstractPropertyOwner::Kind ObjectTreeView::displayed_kinds() const
{
  return AbstractPropertyOwner::Kind::Object;
}

void ObjectTreeView::remove_selected_tags(Object& object) const
{
  auto& scene = model()->scene();
  auto selection = scene.selected_tags();
  selection = ::filter_if(selection, [&object](const auto* tag) {
    return tag->owner() == &object;
  });
  using remove_command_type = RemoveCommand<List<Tag>>;
  model()->scene().submit<remove_command_type>(object.tags, selection);
  scene.invalidate();
}

}  // namespace omm
