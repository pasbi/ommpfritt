#include "managers/objectmanager/objecttreeview.h"

#include <memory>
#include <glog/logging.h>
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>
#include <QDrag>

#include "menuhelper.h"
#include "scene/contextes.h"
#include "commands/removecommand.h"
#include "commands/addcommand.h"
#include "renderers/style.h"
#include "scene/scene.h"
#include "commands/propertycommand.h"
#include "properties/referenceproperty.h"
#include "common.h"
#include "scene/propertyownermimedata.h"
#include "tags/tag.h"
#include "mainwindow/application.h"
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
  , m_model(model)
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

  const auto selected_tags = AbstractPropertyOwner::cast<Tag>(this->selected_tags());
  const auto selected_objects = AbstractPropertyOwner::cast<Object>(this->selected_objects());

  auto& remove_action = action(menu, tr("&remove"), *this, &ManagerItemView::remove_selection);
  remove_action.setEnabled(selected_tags.size() > 0 || selected_objects.size() > 0);

  auto tag_menu = std::make_unique<QMenu>(tr("&attach tag"));
  for (const auto& key : Tag::keys()) {
    action(*tag_menu, QString::fromStdString(key), [this, key, selected_objects](){
      Scene& scene = model()->scene;
      scene.undo_stack.beginMacro(tr("Add Tag"));
      for (auto&& object : selected_objects) {
        using AddTagCommand = omm::AddCommand<omm::List<omm::Tag>>;
        scene.submit<AddTagCommand>(object->tags, Tag::make(key, *object));
      }
      scene.undo_stack.endMacro();
    });
  }
  tag_menu->setEnabled(selected_objects.size() > 0);
  menu.addMenu(tag_menu.release());

  const auto convertables = ::filter_if(selected_objects, [](const Object* object) {
    return !!(object->flags() & Object::Flag::Convertable);
  });

  auto& app = Application::instance();
  menu.addAction(app.key_bindings.make_action(app, "convert objects").release());
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
  return AbstractPropertyOwner::cast(m_selection_model->selected_tags());
}

bool ObjectTreeView::remove_selection()
{
  const auto selected_tags = AbstractPropertyOwner::cast<Tag>(this->selected_tags());
  const auto selected_objects = AbstractPropertyOwner::cast<Object>(this->selected_objects());

  const auto accumulate_free_tags = [selected_objects](auto map, auto* tag) {
    if (!::contains(selected_objects, tag->owner)) {
      map[tag->owner].insert(tag);
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

  Scene& scene = model()->scene;
  std::set<Property*> properties;
  if (removed_items.size() > 0 && can_remove_selection(this, scene, removed_items, properties))
  {
    scene.undo_stack.beginMacro("Remove Selection");
    if (properties.size() > 0) {
      using command_type = PropertiesCommand<ReferenceProperty>;
      scene.template submit<command_type>(properties, nullptr);
    }
    remove(scene, model()->structure, selected_objects);
    for (auto&& item : explicitely_removed_tags) {
      remove(scene, item.first->tags, item.second);
    }
    scene.undo_stack.endMacro();
    return true;
  } else {
    return false;
  }
}

void ObjectTreeView::handle_drag_event(QDragMoveEvent* e)
{
  m_model.current_tag_predecessor = m_tags_item_delegate->tag_before(e->pos());
  m_model.current_tag = m_tags_item_delegate->tag_at(e->pos());
  setDropIndicatorShown(indexAt(e->pos()).column() != ObjectTreeAdapter::TAGS_COLUMN);
}

void ObjectTreeView::dragEnterEvent(QDragEnterEvent* e)
{
  handle_drag_event(e);
  ManagerItemView::dragEnterEvent(e);
}

void ObjectTreeView::dragMoveEvent(QDragMoveEvent* e)
{
  handle_drag_event(e);
  ManagerItemView::dragMoveEvent(e);
}

void ObjectTreeView::paintEvent(QPaintEvent* e)
{
  const auto n_tags = m_model.max_number_of_tags_on_object();
  const auto tags_width = m_tags_item_delegate->tag_icon_size().width();
  setColumnWidth(ObjectTreeAdapter::TAGS_COLUMN, n_tags * tags_width);
  ManagerItemView<QTreeView, ObjectTreeAdapter>::paintEvent(e);
}

}  // namespace omm
