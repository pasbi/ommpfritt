#include "managers/objectmanager/objectmanager.h"
#include "managers/objectmanager/objecttreeview.h"
#include "scene/scene.h"
#include <functional>
#include "mainwindow/application.h"
#include <QCoreApplication>
#include "keybindings/defaultkeysequenceparser.h"
#include "tags/tag.h"
#include "commands/addcommand.h"
#include "commands/movecommand.h"
#include "objects/empty.h"

namespace omm
{

ObjectManager::ObjectManager(Scene& scene)
  : ItemManager( QCoreApplication::translate("any-context", "ObjectManager"),
                 scene, scene.object_tree )
{
  setObjectName(TYPE);
  connect(&scene.message_box, SIGNAL(selection_changed(std::set<AbstractPropertyOwner*>)),
          &item_view(), SLOT(set_selection(std::set<AbstractPropertyOwner*>)));
}

std::vector<CommandInterface::ActionInfo<ObjectManager>> ObjectManager::action_infos()
{
  DefaultKeySequenceParser parser("://default_keybindings.cfg", "ObjectManager");
  const auto ai = [parser](const std::string& name, const std::function<void(ObjectManager&)>& f) {
    return ActionInfo(name, parser.get_key_sequence(name), f);
  };

  return {
    ai(QT_TRANSLATE_NOOP("any-context", "remove objects and tags"), [](ObjectManager& om) {
      om.scene().remove(&om, om.item_view().selected_items());
    }),
    ai(QT_TRANSLATE_NOOP("any-context", "group objects"), [](ObjectManager&) {
      Application::instance().insert_object(Empty::TYPE, Application::InsertionMode::AsParent);
    })
  };
}

std::vector<std::string> ObjectManager::application_actions() const
{
  std::list<std::string> entries {
    "object/convert objects",
  };

  for (const std::string& key : Tag::keys()) {
    entries.push_back("object/attach/" + key);
  }

  return std::vector(entries.begin(), entries.end());
}

bool ObjectManager::child_key_press_event(QWidget &child, QKeyEvent &event)
{
  Q_UNUSED(child)
  return Application::instance().key_bindings.call(event, *this);
}

void ObjectManager::populate_menu(QMenu &menu)
{
  Application::instance().key_bindings.populate_menu(menu, *this);
}

std::string ObjectManager::type() const { return TYPE; }

}  // namespace omm
