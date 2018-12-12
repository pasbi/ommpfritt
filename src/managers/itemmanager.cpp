#include "managers/itemmanager.h"

#include <QItemSelectionModel>
#include <QEvent>
#include "scene/scene.h"
#include "managers/stylemanager/stylelistview.h"
#include "scene/stylelistadapter.h"
#include "managers/objectmanager/objecttreeview.h"
#include "scene/objecttreeadapter.h"
#include "renderers/style.h"
#include "objects/object.h"

namespace omm
{

template<typename ItemViewT> ItemManager<ItemViewT>
::ItemManager(const QString& title, Scene& scene, item_model_type& model)
  : Manager(title, scene)
{
   auto item_view = std::make_unique<ItemViewT>(model);

  // connect( m_item_view.selectionModel(), &QItemSelectionModel::selectionChanged,
  //          this, &ItemManager::on_selection_changed );

  // item_view->installEventFilter(this);

  setWidget(item_view.release());
}

// template<typename ItemViewT> bool
// ItemManager<ItemViewT>::eventFilter(QObject* object, QEvent* event)
// {
//   if (object == widget() && event->type() == QEvent::MouseButtonRelease) {
//     m_scene.selection_changed();
//   }
//   return Manager::eventFilter(object, event);
// }

// template<typename ItemViewT> void
// ItemManager<ItemViewT>::on_selection_changed( const QItemSelection& selection,
//                                                           const QItemSelection& old_selection )
// {
//   for (auto& index : old_selection.indexes()) {
//     if (!selection.contains(index)) {
//       m_item_model.item_at(index).deselect();
//     }
//   }
//   for (auto& index : selection.indexes()) {
//     m_item_model.item_at(index).select();
//   }
// }

template class ItemManager<ObjectTreeView>;
template class ItemManager<StyleListView>;

}  // namespace omm
