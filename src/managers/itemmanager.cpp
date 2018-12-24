#include "managers/itemmanager.h"

#include <QItemSelectionModel>
#include <QEvent>
#include "scene/scene.h"
#include "managers/stylemanager/stylelistview.h"
#include "scene/listadapter.h"
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
  m_item_view = item_view.get();
  item_view->installEventFilter(this);

  set_widget(std::move(item_view));
}

template<typename ItemViewT> bool
ItemManager<ItemViewT>::eventFilter(QObject* object, QEvent* event)
{
  if (object == m_item_view && event->type() == QEvent::MouseButtonRelease) {
    m_scene.set_selection(m_item_view->selected_items());
  }
  return Manager::eventFilter(object, event);
}

template class ItemManager<ObjectTreeView>;
template class ItemManager<StyleListView>;

}  // namespace omm
