#include "managers/objectmanager/objectmanager.h"
#include "managers/propertymanager/propertymanager.h"
#include "managers/stylemanager/stylemanager.h"

namespace omm
{

void register_managers()
{
#define REGISTER_MANAGER(TYPE) Manager::register_type<TYPE>(#TYPE);

  REGISTER_MANAGER(ObjectManager);
  REGISTER_MANAGER(PropertyManager);
  REGISTER_MANAGER(StyleManager);

#undef REGISTER_MANAGER
}

}  // namespace omm


