#include "managers/objectmanager/objecttreeview.h"

#include <memory>
#include <glog/logging.h>
#include <QMenu>
#include <QContextMenuEvent>
#include <QMessageBox>

#include "menuhelper.h"
#include "managers/objectmanager/objecttreeadapter.h"
#include "commands/removeobjectscommand.h"
#include "tags/tag.h"
#include "commands/propertycommand.h"
#include "commands/attachtagcommand.h"
#include "properties/referenceproperty.h"

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

void ObjectTreeView::remove_selected()
{
  auto& scene = model()->scene();
  const auto selection = scene.selected_objects();
  std::map<const Object*, std::set<ReferenceProperty*>> reference_holder_map;
  for (const Object* reference : selection) {
    const auto reference_holders = scene.find_reference_holders(*reference);
    if (reference_holders.size() > 0) {
      reference_holder_map.insert(std::make_pair(reference, reference_holders));
    }
  }

  if (reference_holder_map.size() > 0) {
    const auto message = tr("There are %1 objects being referenced by other objects.\n"
                            "Remove the refrenced objects anyway?")
                          .arg(reference_holder_map.size());
    const auto decision = QMessageBox::warning( this, tr("Warning"), message,
                                                QMessageBox::YesToAll | QMessageBox::Cancel );
    switch (decision) {
    case QMessageBox::YesToAll:
    {
      const auto f = [](std::set<ReferenceProperty*> accu, const auto& v) {
        accu.insert(v.second.begin(), v.second.end());
        return accu;
      };
      const auto properties = std::accumulate( reference_holder_map.begin(),
                                               reference_holder_map.end(),
                                               std::set<ReferenceProperty*>(), f );
      scene.submit<PropertiesCommand<ReferenceProperty>>(properties, nullptr);
      break;
    }
    case QMessageBox::Cancel:
      return;
    default:
      assert(false);
    }
  }
  scene.submit<RemoveObjectsCommand>(scene, selection);
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
