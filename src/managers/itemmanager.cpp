#include "managers/itemmanager.h"

#include <QItemSelectionModel>
#include <QEvent>
#include "scene/scene.h"
#include "managers/stylemanager/stylelistview.h"
#include "managers/stylemanager/stylelistadapter.h"
#include "managers/objectmanager/objecttreeview.h"
#include "managers/objectmanager/objecttreeadapter.h"
#include "renderers/style.h"
#include "objects/object.h"

namespace omm
{

template<typename ItemViewT, typename ItemModelT>
ItemManager<ItemViewT, ItemModelT>::ItemManager(const QString& title, Scene& scene)
  : Manager(title, scene)
  , m_item_model(scene)
  , m_item_view(m_item_model)
{
  connect( m_item_view.selectionModel(), &QItemSelectionModel::selectionChanged,
           this, &ItemManager::on_selection_changed );

  m_item_view.installEventFilter(this);

  // we must not release ownership to Qt.
  // `m_item_view` must be deleted before `m_item_model` is deleted.
  setWidget(&m_item_view);
}

template<typename ItemViewT, typename ItemModelT> bool
ItemManager<ItemViewT, ItemModelT>::eventFilter(QObject* object, QEvent* event)
{
  if (object == widget() && event->type() == QEvent::MouseButtonRelease) {
    m_scene.selection_changed();
  }
  return Manager::eventFilter(object, event);
}

template<typename ItemViewT, typename ItemModelT> void
ItemManager<ItemViewT, ItemModelT>::on_selection_changed( const QItemSelection& selection,
                                                          const QItemSelection& old_selection )
{
  for (auto& index : old_selection.indexes()) {
    if (!selection.contains(index)) {
      m_item_model.item_at(index).deselect();
    }
  }
  for (auto& index : selection.indexes()) {
    m_item_model.item_at(index).select();
  }
}

template class ItemManager<ObjectTreeView, ObjectTreeAdapter>;
template class ItemManager<StyleListView, StyleListAdapter>;

}  // namespace omm
