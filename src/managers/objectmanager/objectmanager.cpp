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
    { QT_TR_NOOP("remove objects and tags"), QKeySequence(tr("Del")) }
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
    { "remove objects and tags", [this]() {
      scene().remove(this, item_view().selected_items());
    } },
  });
}

void ObjectManager::keyPressEvent(QKeyEvent* event)
{
  if (!Application::instance().key_bindings.call(*event, *this)) {
    Manager::keyPressEvent(event);
  }
}

std::vector<std::string> ObjectManager::application_actions() const
{
  return MainWindow::object_menu_entries();
}

}  // namespace omm
