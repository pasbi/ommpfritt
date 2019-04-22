#include "managers/objectmanager/objectmanager.h"
#include "managers/objectmanager/objecttreeview.h"
#include "scene/objecttreeadapter.h"
#include "scene/scene.h"
#include <functional>
#include "mainwindow/application.h"
#include <QCoreApplication>

namespace omm
{

ObjectManager::ObjectManager(Scene& scene)
  : ItemManager( QCoreApplication::translate("any-context", "ObjectManager"),
                 scene, scene.object_tree_adapter )
{
  setObjectName(TYPE);
  connect(&scene, SIGNAL(selection_changed(std::set<AbstractPropertyOwner*>)),
          &item_view(), SLOT(set_selection(std::set<AbstractPropertyOwner*>)));
}

std::vector<CommandInterface::ActionInfo<ObjectManager>> ObjectManager::action_infos()
{
  using AI = ActionInfo<ObjectManager>;
  return {
    AI( QT_TRANSLATE_NOOP("any-context", "remove objects and tags"), QKeySequence("Del"),
      [](ObjectManager& om) { om.scene().remove(&om, om.item_view().selected_items()); } ),
  };
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

std::string ObjectManager::type() const { return TYPE; }

}  // namespace omm
