#include "managers/dopesheet/dopesheetmanager.h"
#include "managers/dopesheet/dopesheetview.h"
#include "scene/scene.h"
#include "animation/animator.h"

namespace omm
{

DopeSheetManager::DopeSheetManager(Scene &scene)
  : Manager(tr("Dope Sheet"), scene)
{
  setObjectName(TYPE);
  auto dope_sheet_view = std::make_unique<DopeSheetView>(scene.animator());
  m_dope_sheet_view = dope_sheet_view.get();
  set_widget(std::move(dope_sheet_view));
  m_dope_sheet_view->setModel(&m_scene.animator());
}

bool DopeSheetManager::perform_action(const std::string& name)
{
  LINFO << name;
  return false;
}

}
