#include "managers/objectmanager/objectmanager.h"
#include "commands/addcommand.h"
#include "commands/movecommand.h"
#include "keybindings/keybindings.h"
#include "main/application.h"
#include "managers/objectmanager/objecttreeview.h"
#include "objects/empty.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include "tags/tag.h"
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <functional>

namespace omm
{
ObjectManager::ObjectManager(Scene& scene)
    : ItemManager(QCoreApplication::translate("any-context", "ObjectManager"),
                  scene,
                  scene.object_tree())
{
  connect(&scene.mail_box(),
          &MailBox::selection_changed,
          &item_view(),
          &ObjectTreeView::set_selection);
}

bool ObjectManager::perform_action(const QString& name)
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
  KeyBindings& kb = *app.key_bindings;
  const bool object_selected = !item_view().selected_objects().empty();

  const auto e_os = [object_selected](QAction* action) {
    action->setEnabled(object_selected);
    return action;
  };
  QMenu menu(QCoreApplication::translate("any-context", ObjectManager::TYPE));
  menu.addAction(e_os(kb.make_menu_action(app, "convert objects").release()));
  menu.addAction(e_os(kb.make_menu_action(*this, "remove objects and tags").release()));
  menu.addAction(e_os(kb.make_menu_action(*this, "group objects").release()));
  QMenu attach_menu(tr("Attach"));
  menu.addMenu(&attach_menu);
  for (const QString& tag_type : Tag::keys()) {
    attach_menu.addAction(e_os(kb.make_menu_action(app, tag_type).release()));
  }

  menu.exec(event->globalPos());
}

QString ObjectManager::type() const
{
  return TYPE;
}

}  // namespace omm
