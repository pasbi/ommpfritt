#include "managers/manageritemview.h"

#include <QMessageBox>
#include <QContextMenuEvent>
#include <QMenu>
#include "renderers/style.h"
#include "commands/propertycommand.h"
#include "commands/removecommand.h"
#include "properties/referenceproperty.h"

#include "managers/stylemanager/stylelistview.h"
#include "scene/stylelistadapter.h"
#include "managers/objectmanager/objecttreeview.h"
#include "scene/objecttreeadapter.h"
#include "scene/scene.h"

namespace omm
{

template<typename ItemViewT, typename ItemModelT>
ManagerItemView<ItemViewT, ItemModelT>::ManagerItemView(ItemModelT& model)
{
  this->setSelectionMode(QAbstractItemView::ExtendedSelection);
  this->setDragEnabled(true);
  this->setDefaultDropAction(Qt::MoveAction);
  this->viewport()->setAcceptDrops(true);

  ItemViewT::setModel(&model);
}

template<typename ItemViewT, typename ItemModelT>
ManagerItemView<ItemViewT, ItemModelT>::~ManagerItemView()
{
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

template<typename ItemViewT, typename ItemModelT> bool
ManagerItemView<ItemViewT, ItemModelT>::remove_selection()
{
  using item_type = typename ItemModelT::item_type;
  static_assert( std::is_base_of<AbstractPropertyOwner, item_type>::value,
                 "item_type must be a property owner." );

  const auto selection = AbstractPropertyOwner::cast<item_type>(selected_items());

  std::map<const AbstractPropertyOwner*, std::set<ReferenceProperty*>> reference_holder_map;
  for (const auto* reference : selection) {
    const auto reference_holders = model()->scene().find_reference_holders(*reference);
    if (reference_holders.size() > 0) {
      reference_holder_map.insert(std::make_pair(reference, reference_holders));
    }
  }

  if (reference_holder_map.size() > 0) {
    const auto message = QObject::tr("There are %1 items being referenced by other items.\n"
                                     "Remove the refrenced items anyway?")
                                    .arg(reference_holder_map.size());
    const auto decision = QMessageBox::warning( this, QObject::tr("Warning"), message,
                                                QMessageBox::YesToAll | QMessageBox::Cancel );
    switch (decision) {
    case QMessageBox::YesToAll:
    {
      const auto merge = [](std::set<ReferenceProperty*> accu, const auto& v) {
        accu.insert(v.second.begin(), v.second.end());
        return accu;
      };
      const auto properties = std::accumulate( reference_holder_map.begin(),
                                               reference_holder_map.end(),
                                               std::set<ReferenceProperty*>(), merge );
      model()->scene().template submit<PropertiesCommand<ReferenceProperty>>(properties, nullptr);
      break;
    }
    case QMessageBox::Cancel:
      return false;
    default:
      assert(false);
    }
  }

  using remove_command_type = RemoveCommand<typename ItemModelT::structure_type>;
  model()->scene().template submit<remove_command_type>(model()->structure(), selection);
  return true;
}

template<typename ItemViewT, typename ItemModelT>
void ManagerItemView<ItemViewT, ItemModelT>::mouseReleaseEvent(QMouseEvent* e)
{
  e->ignore();
  ItemViewT::mouseReleaseEvent(e);
}

template<typename ItemViewT, typename ItemModelT>
std::set<AbstractPropertyOwner*> ManagerItemView<ItemViewT, ItemModelT>::selected_items() const
{
  const auto get_object = [this](const QModelIndex& index) {
    return &model()->item_at(index);
  };

  const auto selected_indexes = this->selectionModel()->selectedIndexes();
  return ::transform<AbstractPropertyOwner*, std::set>(selected_indexes, get_object);
}

template class ManagerItemView<QListView, StyleListAdapter>;
template class ManagerItemView<QTreeView, ObjectTreeAdapter>;

}  // namespace omm
