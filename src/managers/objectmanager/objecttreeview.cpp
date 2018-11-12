#include "managers/objectmanager/objecttreeview.h"

#include <memory>
#include <glog/logging.h>
#include <QMenu>
#include <QContextMenuEvent>

#include "menuhelper.h"
#include "managers/objectmanager/objecttreeadapter.h"
#include "commands/removeobjectscommand.h"

namespace
{

const omm::Object* object_at(const omm::ObjectTreeView& view, const QPoint& pos)
{
  const auto index = view.indexAt(pos);
  const auto& object = view.model().object_at(index);
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
{
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setDragEnabled(true);
  setDefaultDropAction(Qt::MoveAction);
  viewport()->setAcceptDrops(true);
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


void ObjectTreeView::set_model(ObjectTreeAdapter& model)
{
  QTreeView::setModel(&model);
}

ObjectTreeAdapter& ObjectTreeView::model() const
{
  return static_cast<ObjectTreeAdapter&>(*QTreeView::model());
}

void ObjectTreeView::populate_menu(QMenu& menu, const Object& subject) const
{
  action(menu, tr("&remove"), *this, &ObjectTreeView::remove_selected);
}

ObjectRefs ObjectTreeView::selection() const
{
  const auto object_at = [this](const QModelIndex& index) -> Object& {
    return model().object_at(index);
  };
  const auto indexes = selectionModel()->selectedRows();
  return ::transform<ObjectRef, std::vector>(indexes, object_at);
}

void ObjectTreeView::remove_selected() const
{
  Scene& scene = model().scene();
  scene.submit<RemoveObjectsCommand>(scene, selection());
}


}  // namespace
