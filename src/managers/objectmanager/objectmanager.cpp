#include "managers/objectmanager/objectmanager.h"
#include "managers/objectmanager/objecttreeview.h"
#include "scene/objecttreeadapter.h"
#include "scene/scene.h"
#include <functional>
#include "mainwindow/application.h"

#include <glog/logging.h>

namespace omm
{

std::map<std::string, QKeySequence> ObjectManager::default_bindings()
{
  return {
    { "delete", QKeySequence("Del") }
  };
}

ObjectManager::ObjectManager(Scene& scene)
  : ItemManager(tr("Objects"), scene, scene.object_tree_adapter)
{
  setWindowTitle(tr("object manager"));
  setObjectName(TYPE);
}

void ObjectManager::call(const std::string& command)
{
  dispatch(command, {
    { "delete", std::bind(&ObjectTreeView::remove_selection, &item_view()) }
  });
}

void ObjectManager::keyPressEvent(QKeyEvent* event)
{
  if (!Application::instance().key_bindings.call(*event, *this)) {
    Manager::keyPressEvent(event);
  }
}

}  // namespace omm
