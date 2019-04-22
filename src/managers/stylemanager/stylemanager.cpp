#include "managers/stylemanager/stylemanager.h"

#include <QEvent>
#include "renderers/style.h"
#include "managers/stylemanager/stylelistview.h"
#include "scene/listadapter.h"
#include "scene/scene.h"
#include "commands/addcommand.h"
#include "mainwindow/application.h"
#include <QCoreApplication>

namespace omm
{


StyleManager::StyleManager(Scene& scene)
  : ItemManager( QCoreApplication::translate("any-context", "StyleManager"),
                 scene, scene.style_list_adapter )
{
  setObjectName(TYPE);
  connect(&scene, SIGNAL(style_selection_changed(std::set<Style*>)),
          &item_view(), SLOT(set_selection(std::set<Style*>)));
}

std::vector<CommandInterface::ActionInfo<StyleManager>> StyleManager::action_infos()
{
  using AI = ActionInfo<StyleManager>;
  return {
    AI( QT_TRANSLATE_NOOP("any-context", "remove styles"), QKeySequence(), [](StyleManager& sm) {
      sm.scene().remove(&sm, sm.item_view().selected_items());
    }),
  };
}

void StyleManager::keyPressEvent(QKeyEvent* event)
{
  if (!Application::instance().key_bindings.call(*event, *this)) {
    Manager::keyPressEvent(event);
  }
}

std::vector<std::string> StyleManager::application_actions() const
{
  return { "new style" };
}

void StyleManager::populate_menu(QMenu& menu)
{
  auto& key_bindings = Application::instance().key_bindings;
  menu.addAction(key_bindings.make_action(*this, "remove styles").release());
}

std::string StyleManager::type() const { return TYPE; }

}  // namespace omm
