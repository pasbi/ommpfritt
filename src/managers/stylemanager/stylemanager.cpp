#include "managers/stylemanager/stylemanager.h"

#include <QEvent>
#include "renderers/style.h"
#include "managers/stylemanager/stylelistview.h"
#include "scene/listadapter.h"
#include "scene/scene.h"
#include "commands/addcommand.h"
#include "mainwindow/application.h"

namespace omm
{


std::map<std::string, QKeySequence> StyleManager::default_bindings()
{
  return {
    { QT_TR_NOOP("remove styles"), QKeySequence(tr("Del")) }
  };
}

StyleManager::StyleManager(Scene& scene)
  : ItemManager(tr("Styles"), scene, scene.style_list_adapter)
{
  setWindowTitle(tr("style manager"));
  setObjectName(TYPE);
}

void StyleManager::call(const std::string& command)
{
  dispatch(command, {
    { "remove styles", [this](){ scene().remove(this, item_view().selected_items()); } },
  });
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

}  // namespace omm
