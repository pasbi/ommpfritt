#include "managers/objectmanager/objecttreeview.h"

#include <memory>
#include <glog/logging.h>

#include "menuhelper.h"
#include "commands/addtagcommand.h"
#include "scene/contextes.h"
#include "commands/removecommand.h"
#include "renderers/style.h"
#include "scene/scene.h"
#include "commands/propertycommand.h"
#include "properties/referenceproperty.h"

namespace
{

template<typename StructureT>
void remove( omm::Scene& scene, StructureT& structure,
             const std::set<typename StructureT::item_type*>& selection )
{
  scene.template submit<omm::RemoveCommand<StructureT>>(structure, selection);
}

}  // namespace

namespace omm
{

ObjectTreeView::ObjectTreeView(ObjectTreeAdapter& model)
  : ManagerItemView(model)
  , m_selection_model(std::make_unique<ObjectTreeSelectionModel>(model).release())
  , m_tags_item_delegate(std::make_unique<TagsItemDelegate>(*this, *m_selection_model))
{
  setItemDelegateForColumn(2, m_tags_item_delegate.get());
  setSelectionModel(m_selection_model.get());
  setSelectionBehavior(QAbstractItemView::SelectItems);
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
      action(*tag_menu, QString::fromStdString(key), [this, key, object](){
        model()->scene().submit<AddTagCommand>(*object, Tag::make(key));
      });
    }
    menu.addMenu(tag_menu.release());
  }
}

std::set<AbstractPropertyOwner*> ObjectTreeView::selected_items() const
{
  return ::merge(selected_objects(), selected_tags());
}

std::set<AbstractPropertyOwner*> ObjectTreeView::selected_objects() const
{
  return ManagerItemView::selected_items();
}

std::set<AbstractPropertyOwner*> ObjectTreeView::selected_tags() const
{
  const auto cast = [](Tag* t) { return static_cast<AbstractPropertyOwner*>(t); };
  return ::transform<AbstractPropertyOwner*>(m_selection_model->selected_tags(), cast);
}

bool ObjectTreeView::remove_selection()
{
  const auto selected_tags = AbstractPropertyOwner::cast<Tag>(this->selected_tags());
  const auto selected_objects = AbstractPropertyOwner::cast<Object>(this->selected_objects());

  const auto accumulate_free_tags = [selected_objects](auto map, auto* tag) {
    auto* owner = tag->owner();
    if (selected_objects.count(owner) == 0) {
      map[owner].insert(tag);
    }
    return map;
  };
  const auto explicitely_removed_tags = std::accumulate( selected_tags.begin(),
                                                         selected_tags.end(),
                                                         std::map<Object*, std::set<Tag*>>(),
                                                         accumulate_free_tags );

  const auto accumlate_object_tags = [](std::set<Tag*> tags, const Object* object) {
    std::set<Tag*> object_tags = object->tags.items();
    tags.insert(object_tags.begin(), object_tags.end());
    return tags;
  };
  const auto implicitely_removed_tags = std::accumulate( selected_objects.begin(),
                                                         selected_objects.end(),
                                                         std::set<Tag*>(),
                                                         accumlate_object_tags );

  auto removed_items = ::merge( std::set<AbstractPropertyOwner*>(),
                                selected_objects,
                                implicitely_removed_tags );
  for (auto&& item : explicitely_removed_tags) {
    removed_items.insert(item.second.begin(), item.second.end());
  }

  Scene& scene = model()->scene();
  std::set<Property*> properties;
  if (removed_items.size() > 0 && can_remove_selection(this, scene, removed_items, properties))
  {
    scene.undo_stack().beginMacro("Remove Selection");
    if (properties.size() > 0) {
      using command_type = PropertiesCommand<ReferenceProperty::value_type>;
      scene.template submit<command_type>(properties, nullptr);
    }
    remove(scene, model()->structure(), selected_objects);
    for (auto&& item : explicitely_removed_tags) {
      remove(scene, item.first->tags, AbstractPropertyOwner::cast<Tag>(item.second));
    }
    scene.undo_stack().endMacro();
    return true;
  } else {
    return false;
  }
}

}  // namespace omm
