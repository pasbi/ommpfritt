#include "managers/objectmanager/objectmanager.h"
#include "managers/objectmanager/objecttreeview.h"

#include <glog/logging.h>

namespace omm
{

ObjectManager::ObjectManager(Scene& scene)
  : ItemManager(tr("Objects"), scene)
{
  setWindowTitle(tr("object manager"));
  setObjectName(TYPE());
}

void ObjectManager::on_selection_changed( const QItemSelection& selection,
                                          const QItemSelection& old_selection )
{
  for (auto& index : old_selection.indexes()) {
    if (!selection.contains(index)) {
      m_item_model.object_at(index).deselect();
    }
  }
  for (auto& index : selection.indexes()) {
    m_item_model.object_at(index).select();
  }
}

}  // namespace omm
