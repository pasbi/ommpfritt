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
#include "mainwindow/actions.h"
#include "keybindings/menu.h"

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
  if (name == "remove objects and tags") {
    scene().remove(this, item_view().selected_items());
  } else if (name == "group objects") {
    Application::instance().insert_object(Empty::TYPE, Application::InsertionMode::AsParent);
  } else {
    return false;
  }
  return true;
}

void ObjectManager::contextMenuEvent(QContextMenuEvent* event)
{
  Application& app = Application::instance();
  KeyBindings& kb = app.key_bindings;
  const bool object_selected = !item_view().selected_objects().empty();

  const auto e_os = [object_selected](QAction* action) {
    action->setEnabled(object_selected);
    return action;
  };
  Menu menu(QCoreApplication::translate("any-context", ObjectManager::TYPE));
  menu.addAction(e_os(kb.make_action(app, "convert objects").release()));
  menu.addAction(e_os(kb.make_action(*this, "remove objects and tags").release()));
  menu.addAction(e_os(kb.make_action(*this, "group objects").release()));
  Menu attach_menu(tr("Attach"));
  menu.addMenu(&attach_menu);
  for (const std::string& tag_type : Tag::keys()) {
    attach_menu.addAction(e_os(kb.make_action(app, tag_type).release()));
  }

  menu.exec(event->globalPos());
}

std::string ObjectManager::type() const { return TYPE; }

}  // namespace omm
