#include "registers.h"
#include "logging.h"

#include "register_managers.cpp"
#include "register_objects.cpp"
#include "register_tags.cpp"
#include "register_properties.cpp"
#include "register_tools.cpp"
#include "register_nodes.cpp"

namespace omm
{

void register_everything()
{
  register_managers();
  register_objects();
  register_tags();
  register_properties();
  register_tools();
  register_nodes();
}

}  // namespace omm
