#include <glog/logging.h>

#include "managers/objectmanager.h"
#include "scene/sceneobjectmodel.h"

omm::ObjectManager::ObjectManager(omm::Scene& scene)
  : Manager(scene)
  , m_columns(scene)
{
  // Glib::RefPtr<Gtk::TreeStore> tree_model = Gtk::TreeStore::create(m_columns);
  // m_tree_view.set_model(tree_model);
  // add(m_tree_view);
}