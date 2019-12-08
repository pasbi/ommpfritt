#include "managers/objectmanager/objectmanager.h"
#include "managers/nodemanager/nodemanager.h"
#include "managers/propertymanager/propertymanager.h"
#include "managers/stylemanager/stylemanager.h"
#include "managers/pythonconsole/pythonconsole.h"
#include "managers/historymanager/historymanager.h"
#include "managers/boundingboxmanager/boundingboxmanager.h"
#include "managers/timeline/timeline.h"
#include "managers/curvemanager/curvemanager.h"
#include "dopesheet/dopesheetmanager.h"

namespace omm
{

void register_managers()
{
#define REGISTER_MANAGER(TYPE) Manager::register_type<TYPE>(#TYPE)

  REGISTER_MANAGER(DopeSheetManager);
  REGISTER_MANAGER(CurveManager);
  REGISTER_MANAGER(ObjectManager);
  REGISTER_MANAGER(PropertyManager);
  REGISTER_MANAGER(StyleManager);
  REGISTER_MANAGER(PythonConsole);
  REGISTER_MANAGER(HistoryManager);
  REGISTER_MANAGER(BoundingBoxManager);
  REGISTER_MANAGER(TimeLine);
  REGISTER_MANAGER(NodeManager);

#undef REGISTER_MANAGER
}

}  // namespace omm


