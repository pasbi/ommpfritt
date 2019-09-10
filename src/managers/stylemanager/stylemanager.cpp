#include "managers/stylemanager/stylemanager.h"

#include <QEvent>
#include "renderers/style.h"
#include "managers/stylemanager/stylelistview.h"
#include "scene/scene.h"
#include "commands/addcommand.h"
#include "mainwindow/application.h"
#include <QCoreApplication>
#include "keybindings/defaultkeysequenceparser.h"
#include "scene/messagebox.h"

namespace omm
{


StyleManager::StyleManager(Scene& scene)
  : ItemManager( QCoreApplication::translate("any-context", "StyleManager"),
                 scene, scene.styles())
{
  setObjectName(TYPE);
  connect(&scene.message_box(), SIGNAL(selection_changed(std::set<Style*>)),
          &item_view(), SLOT(set_selection(std::set<Style*>)));
}

std::vector<CommandInterface::ActionInfo<StyleManager>> StyleManager::action_infos()
{
  DefaultKeySequenceParser parser("://default_keybindings.cfg", "StyleManager");
  const auto ai = [parser](const std::string& name, const std::function<void(StyleManager&)>& f) {
    return ActionInfo(name, parser.get_key_sequence(name), f);
  };

  return {
    ai(QT_TRANSLATE_NOOP("any-context", "remove styles"), [](StyleManager& om) {
      om.scene().remove(&om, om.item_view().selected_items());
    })
  };
}

std::vector<std::string> StyleManager::application_actions() const
{
  return { "new style" };
}

void StyleManager::populate_menu(QMenu& menu)
{
  Application::instance().key_bindings.populate_menu(menu, *this);
}

bool StyleManager::child_key_press_event(QWidget &child, QKeyEvent &event)
{
  Q_UNUSED(child)
  return Application::instance().key_bindings.call(event, *this);
}

std::string StyleManager::type() const { return TYPE; }

}  // namespace omm
