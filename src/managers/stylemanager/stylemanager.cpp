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


const std::map<std::string, QKeySequence> StyleManager::DEFAULT_BINDINGS {
  { "new", QKeySequence("Ctrl+N") },
};

StyleManager::StyleManager(Scene& scene)
  : ItemManager(tr("Styles"), scene, scene.style_list_adapter)
{
  setWindowTitle(tr("style manager"));
  setObjectName(TYPE);
}

void StyleManager::call(const std::string& command)
{
  dispatch(command, {
    { "new", [this]() {
      using command_type = AddCommand<List<Style>>;
      scene().submit<command_type>(scene().styles, scene().default_style().clone());
    } }
  });
}

void StyleManager::keyPressEvent(QKeyEvent* event)
{
  if (!Application::instance().key_bindings.call(*event, *this)) {
    Manager::keyPressEvent(event);
  }
}

}  // namespace omm
