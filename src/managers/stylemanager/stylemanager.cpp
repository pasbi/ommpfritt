#include "managers/stylemanager/stylemanager.h"

#include <QEvent>
#include "renderers/style.h"
#include "managers/stylemanager/stylelistview.h"
#include "scene/scene.h"
#include "commands/addcommand.h"
#include "mainwindow/application.h"
#include <QCoreApplication>
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

std::string StyleManager::type() const { return TYPE; }

bool StyleManager::perform_action(const std::string& action_name)
{
  LINFO << action_name;
  return false;
}

}  // namespace omm
