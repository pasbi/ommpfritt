#include "managers/dopesheet/dopesheetmanager.h"
#include "managers/dopesheet/dopesheetview.h"
#include <memory>

namespace omm
{

DopeSheetManager::DopeSheetManager(Scene &scene)
  : Manager(tr("Dope Sheet"), scene)
  , m_dope_sheet(std::make_unique<DopeSheet>(scene))
{
  setObjectName(TYPE);
  auto dope_sheet_view = std::make_unique<DopeSheetView>();
  m_dope_sheet_view = dope_sheet_view.get();
  set_widget(std::move(dope_sheet_view));
  m_dope_sheet_view->setModel(m_dope_sheet.get());
}

}
