#include "managers/objectmanager/objectmanager.h"
#include "managers/objectmanager/objecttreeview.h"
#include "scene/scene.h"
#include <functional>
#include "mainwindow/application.h"
#include <QCoreApplication>
#include "tags/tag.h"
#include "commands/addcommand.h"
#include "commands/movecommand.h"
#include "objects/empty.h"
#include "scene/messagebox.h"

namespace omm
{

ObjectManager::ObjectManager(Scene& scene)
  : ItemManager( QCoreApplication::translate("any-context", "ObjectManager"),
                 scene, scene.object_tree() )
{
  setObjectName(TYPE);
  connect(&scene.message_box(), SIGNAL(selection_changed(std::set<AbstractPropertyOwner*>)),
          &item_view(), SLOT(set_selection(std::set<AbstractPropertyOwner*>)));
}

bool ObjectManager::perform_action(const std::string& name)
{
  LINFO << name;
  return false;
}

std::string ObjectManager::type() const { return TYPE; }

}  // namespace omm
