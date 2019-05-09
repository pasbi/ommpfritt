#include "managers/objectmanager/objectmanager.h"
#include "managers/objectmanager/objecttreeview.h"
#include "scene/objecttreeadapter.h"
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
                 scene, scene.object_tree_adapter )
{
  setObjectName(TYPE);
  connect(&scene, SIGNAL(selection_changed(std::set<AbstractPropertyOwner*>)),
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
    ai(QT_TRANSLATE_NOOP("any-context", "group objects"), [](ObjectManager& om) {
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

void ObjectManager::group_selected_objects()
{
  auto selected_objects = scene().item_selection<Object>();
  Object::remove_internal_children(selected_objects);
  auto ordererd_selected_objects = Object::sort(selected_objects);

  if (!ordererd_selected_objects.empty()) {
    auto macro = scene().history.start_macro(tr("Group"));

    using add_command_type = AddCommand<Tree<Object>>;
    auto empty = std::make_unique<Empty>(&scene());
    auto& empty_ref = *empty;
    scene().submit<add_command_type>(scene().object_tree, std::move(empty));

    using move_command_type = MoveCommand<Tree<Object>>;
    using move_context = move_command_type::context_type;
    Object* predecessor = nullptr;
    const auto f = [&empty_ref, &predecessor](Object* o) {
      const auto context = move_command_type::context_type(*o, empty_ref, predecessor);
      assert(context.is_sane());
      return context;
    };
    const auto move_contextes = ::transform<move_context>(ordererd_selected_objects, f);
    scene().submit<move_command_type>(scene().object_tree, move_contextes);
  }
}

}  // namespace omm
