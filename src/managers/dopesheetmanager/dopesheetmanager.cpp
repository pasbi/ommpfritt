#include "managers/dopesheetmanager/dopesheetmanager.h"
#include "animation/animator.h"
#include "managers/dopesheetmanager/dopesheetview.h"
#include "scene/scene.h"

namespace omm
{
DopeSheetManager::DopeSheetManager(Scene& scene) : Manager(tr("Dope Sheet"), scene)
{
  auto dope_sheet_view = std::make_unique<DopeSheetView>(scene.animator());
  m_dope_sheet_view = dope_sheet_view.get();
  set_widget(std::move(dope_sheet_view));
}

bool DopeSheetManager::perform_action(const QString& name)
{
  LINFO << name;
  return false;
}

}  // namespace omm
