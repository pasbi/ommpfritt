#include "managers/manageritemview.h"

#include <QMessageBox>
#include <QContextMenuEvent>
#include <QMenu>
#include "renderers/style.h"
#include "commands/propertycommand.h"
#include "commands/removecommand.h"
#include "properties/referenceproperty.h"

#include "managers/stylemanager/stylelistview.h"
#include "scene/listadapter.h"
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
  const auto selection = selected_items();
  Scene& scene = model()->scene();
  std::set<Property*> properties;
  if (selection.size() > 0 && can_remove_selection(this, scene, selection, properties))
  {
    const auto typed_selection = AbstractPropertyOwner::cast<item_type>(selection);
    using remove_command_type = RemoveCommand<typename ItemModelT::structure_type>;
    scene.undo_stack().beginMacro("Remove Selection");
    if (properties.size() > 0) {
      using command_type = PropertiesCommand<ReferenceProperty::value_type>;
      scene.template submit<command_type>(properties, nullptr);
    }
    scene.template submit<remove_command_type>(model()->structure(), typed_selection);
    scene.undo_stack().endMacro();
    return true;
  } else {
    return false;
  }
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

template class ManagerItemView<QListView, ListAdapter<Style>>;
template class ManagerItemView<QTreeView, ObjectTreeAdapter>;

bool can_remove_selection(QWidget* parent, Scene& scene,
                          const std::set<AbstractPropertyOwner*>& selection,
                          std::set<Property*>& properties)
{
  const auto reference_holder_map = scene.find_reference_holders(selection);
  if (reference_holder_map.size() > 0) {
    const auto message = QObject::tr("There are %1 items being referenced by other items.\n"
                                     "Remove the refrenced items anyway?")
                                    .arg(reference_holder_map.size());
    const auto decision = QMessageBox::warning( parent, QObject::tr("Warning"), message,
                                                QMessageBox::YesToAll | QMessageBox::Cancel );
    switch (decision) {
    case QMessageBox::YesToAll:
    {
      const auto merge = [](std::set<Property*> accu, const auto& v) {
        accu.insert(v.second.begin(), v.second.end());
        return accu;
      };
      properties = std::accumulate( reference_holder_map.begin(), reference_holder_map.end(),
                                    std::set<Property*>(), merge );
      return true;
    }
    case QMessageBox::Cancel:
      return false;
    default:
      assert(false);
    }
  } else {
    return true;
  }
}

// TODO undo removing of two top-level siblings fails

}  // namespace omm
