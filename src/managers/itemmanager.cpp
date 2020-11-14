#include "managers/itemmanager.h"

#include "managers/objectmanager/objecttreeview.h"
#include "managers/stylemanager/stylelistview.h"
#include "objects/object.h"
#include "renderers/style.h"
#include "scene/scene.h"
#include <QEvent>
#include <QItemSelectionModel>
#include <QTimer>

namespace omm
{
template<typename ItemViewT>
ItemManager<ItemViewT>::ItemManager(const QString& title, Scene& scene, item_model_type& model)
    : Manager(title, scene)
{
  auto item_view = std::make_unique<ItemViewT>(model);
  m_item_view = item_view.get();
  set_widget(std::move(item_view));
}

template<typename ItemViewT> ItemViewT& ItemManager<ItemViewT>::item_view() const
{
  return *m_item_view;
}

template class ItemManager<ObjectTreeView>;
template class ItemManager<StyleListView>;

}  // namespace omm
