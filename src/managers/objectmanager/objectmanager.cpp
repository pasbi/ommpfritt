#include "managers/objectmanager/objectmanager.h"

#include <glog/logging.h>
#include <QTreeView>
#include <QTimer>

#include "scene/scene.h"

namespace omm
{

ObjectManager::ObjectManager(Scene& scene)
  : Manager(tr("Objects"), scene)
  , m_object_tree_adapter(scene.root())
{
  setWindowTitle(tr("object manager"));

  m_scene.ObserverRegister<AbstractObjectTreeObserver>::register_observer(m_object_tree_adapter);

  auto tree_view = std::make_unique<QTreeView>();
  tree_view->setModel(&m_object_tree_adapter);

  connect( tree_view->selectionModel(), &QItemSelectionModel::selectionChanged,
           this, &ObjectManager::on_selection_changed );

  setWidget(tree_view.release());
  setObjectName(TYPE());
}

ObjectManager::~ObjectManager()
{
  m_scene.ObserverRegister<AbstractObjectTreeObserver>::unregister_observer(m_object_tree_adapter);
}

void ObjectManager::on_selection_changed( const QItemSelection& selection,
                                          const QItemSelection& old_selection )
{
  for (auto& index : old_selection.indexes()) {
    if (!selection.contains(index)) {
      m_object_tree_adapter.object_at(index).deselect();
    }
  }

  for (auto& index : selection.indexes()) {
    m_object_tree_adapter.object_at(index).select();
  }

  m_scene.selection_changed();
}

}  // namespace omm
