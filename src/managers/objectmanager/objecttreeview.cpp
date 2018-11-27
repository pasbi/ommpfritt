#include "managers/objectmanager/objecttreeview.h"

#include <memory>
#include <glog/logging.h>
#include <QMenu>
#include <QContextMenuEvent>

#include "menuhelper.h"
#include "managers/objectmanager/objecttreeadapter.h"
#include "commands/removeobjectscommand.h"
#include "tags/tag.h"
#include "commands/attachtagcommand.h"

namespace
{

const omm::Object* object_at(const omm::ObjectTreeView& view, const QPoint& pos)
{
  const auto index = view.indexAt(pos);
  const auto& object = view.model()->object_at(index);
  if (object.is_root()) {
    return nullptr;
  } else {
    return &object;
  }
}

}  // namespace

namespace omm
{

ObjectTreeView::ObjectTreeView()
  : m_tags_item_delegate(std::make_unique<TagsItemDelegate>(*this))
{
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setDragEnabled(true);
  setDefaultDropAction(Qt::MoveAction);
  viewport()->setAcceptDrops(true);
  setItemDelegateForColumn(2, m_tags_item_delegate.get());
}

ObjectTreeView::~ObjectTreeView()
{
  set_model(nullptr); // unregister observer
}

void ObjectTreeView::contextMenuEvent(QContextMenuEvent *event)
{
  auto object = object_at(*this, event->pos());
  if (object == nullptr) {
    event->ignore();
  } else {
    auto menu = std::make_unique<QMenu>();
    populate_menu(*menu, *object);

    menu->move(event->globalPos());
    menu->show();

    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu.release();
    event->accept();
  }
}

void ObjectTreeView::mouseReleaseEvent(QMouseEvent *event)
{
  QTreeView::mouseReleaseEvent(event);
  Q_EMIT mouse_released();
}

void ObjectTreeView::set_model(ObjectTreeAdapter* model)
{
  if (this->model()) {
    this->model()->scene().Observed<AbstractSelectionObserver>::unregister_observer(*this);
  }
  QTreeView::setModel(model);
  if (this->model()) {
    this->model()->scene().Observed<AbstractSelectionObserver>::register_observer(*this);
  }
}

ObjectTreeAdapter* ObjectTreeView::model() const
{
  return static_cast<ObjectTreeAdapter*>(QTreeView::model());
}

void ObjectTreeView::populate_menu(QMenu& menu, const Object& subject) const
{
  action(menu, tr("&remove"), *this, &ObjectTreeView::remove_selected);
  auto tag_menu = std::make_unique<QMenu>(tr("&attach tag"));
  for (const auto& key : Tag::keys()) {
    action(*tag_menu, QString::fromStdString(key), [this, key](){
      attach_tag_to_selected(key);
    });
  }
  menu.addMenu(tag_menu.release());
}

void ObjectTreeView::remove_selected() const
{
  auto& scene = model()->scene();
  scene.submit<RemoveObjectsCommand>(scene);
}

void ObjectTreeView::attach_tag_to_selected(const std::string& tag_class) const
{
  auto tag = Tag::make(tag_class);
  auto& scene = model()->scene();
  scene.submit<AttachTagCommand>(scene, std::move(tag));
}

void ObjectTreeView::set_selection(const std::set<AbstractPropertyOwner*>& selection, Object& root)
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

void ObjectTreeView::set_selection(const std::set<AbstractPropertyOwner*>& selection)
{
  set_selection(selection, model()->scene().root());
}

}  // namespace omm
