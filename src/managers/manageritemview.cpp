#include "managers/manageritemview.h"

#include <QMessageBox>
#include <QContextMenuEvent>
#include <QMenu>
#include "renderers/style.h"
#include "commands/propertycommand.h"
#include "commands/removeobjectscommand.h"
#include "commands/removestylescommand.h"
#include "properties/referenceproperty.h"

#include "managers/stylemanager/stylelistview.h"
#include "managers/stylemanager/stylelistadapter.h"
#include "managers/objectmanager/objecttreeview.h"
#include "managers/objectmanager/objecttreeadapter.h"

namespace
{

template<typename RemoveCommand, typename PropertyOwner> bool
remove_selection(QWidget& parent, omm::Scene& scene, const std::set<PropertyOwner*>& selection)
{
  std::map<const PropertyOwner*, std::set<omm::ReferenceProperty*>> reference_holder_map;
  for (const PropertyOwner* reference : selection) {
    const auto reference_holders = scene.find_reference_holders(*reference);
    if (reference_holders.size() > 0) {
      reference_holder_map.insert(std::make_pair(reference, reference_holders));
    }
  }

  if (reference_holder_map.size() > 0) {
    const auto message = QObject::tr("There are %1 items being referenced by other items.\n"
                                     "Remove the refrenced items anyway?")
                                    .arg(reference_holder_map.size());
    const auto decision = QMessageBox::warning( &parent, QObject::tr("Warning"), message,
                                                QMessageBox::YesToAll | QMessageBox::Cancel );
    switch (decision) {
    case QMessageBox::YesToAll:
    {
      const auto f = [](std::set<omm::ReferenceProperty*> accu, const auto& v) {
        accu.insert(v.second.begin(), v.second.end());
        return accu;
      };
      const auto properties = std::accumulate( reference_holder_map.begin(),
                                               reference_holder_map.end(),
                                               std::set<omm::ReferenceProperty*>(), f );
      scene.submit<omm::PropertiesCommand<omm::ReferenceProperty>>(properties, nullptr);
      break;
    }
    case QMessageBox::Cancel:
      return false;
    default:
      assert(false);
    }
  }
  scene.submit<RemoveCommand>(scene, selection);
  return true;
}

}  // namespace

namespace omm
{

template<typename ItemViewT, typename ItemModelT>
ManagerItemView<ItemViewT, ItemModelT>::ManagerItemView()
{
  this->setSelectionMode(QAbstractItemView::ExtendedSelection);
  this->setDragEnabled(true);
  this->setDefaultDropAction(Qt::MoveAction);
  this->viewport()->setAcceptDrops(true);
}

template<typename ItemViewT, typename ItemModelT>
ManagerItemView<ItemViewT, ItemModelT>::~ManagerItemView()
{
  set_model(nullptr); // unregister observer
}

template<typename ItemViewT, typename ItemModelT>
void ManagerItemView<ItemViewT, ItemModelT>::set_model(ItemModelT* model)
{
  if (this->model()) {
    this->model()->scene().Observed<AbstractSelectionObserver>::unregister_observer(*this);
  }
  ItemViewT::setModel(model);
  if (this->model()) {
    auto& scene = this->model()->scene();
    scene.Observed<AbstractSelectionObserver>::register_observer(*this);
    QObject::connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, [&scene]() {
      for (Tag* tag : scene.selected_tags()) {
        tag->deselect();
      }
      for (Style* style : scene.selected_styles()) {
        style->deselect();
      }
    });
  }
}

template<typename ItemViewT, typename ItemModelT>
void ManagerItemView<ItemViewT, ItemModelT>::contextMenuEvent(QContextMenuEvent *event)
{
  auto menu = std::make_unique<QMenu>();
  auto& scene = this->model()->scene();

  populate_menu(*menu, this->indexAt(event->pos()));

  menu->move(event->globalPos());
  menu->show();

  menu->setAttribute(Qt::WA_DeleteOnClose);
  menu.release();
  event->accept();
}

template<typename ItemViewT, typename ItemModelT>
ItemModelT* ManagerItemView<ItemViewT, ItemModelT>::model() const
{
  return static_cast<ItemModelT*>(ItemViewT::model());
}

template<typename ItemViewT, typename ItemModelT>
template<typename RemoveCommand, typename PropertyOwner>
bool ManagerItemView<ItemViewT, ItemModelT>
::remove_selection(const std::set<PropertyOwner*>& selection)
{
  return ::remove_selection<RemoveCommand>(*this, this->model()->scene(), selection);
}

template class ManagerItemView<QListView, StyleListAdapter>;
template class ManagerItemView<QTreeView, ObjectTreeAdapter>;

template bool ManagerItemView<QTreeView, ObjectTreeAdapter>
::remove_selection<RemoveObjectsCommand>(const std::set<Object*>& selection);
template bool ManagerItemView<QListView, StyleListAdapter>
::remove_selection<RemoveStylesCommand>(const std::set<Style*>& selection);

}  // namespace omm
