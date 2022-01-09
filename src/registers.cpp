#include "registers.h"
#include "logging.h"
#include <list>

#include "managers/manager.h"
#include "objects/object.h"
#include "tags/tag.h"
#include "properties/property.h"
#include "tools/tool.h"
#include "nodesystem/node.h"

namespace omm_generated
{
void register_managers();
void register_nodes();
void register_objects();
void register_properties();
void register_tags();
void register_tools();
}  // namespace omm_generated

namespace
{
template<typename Ts> void merge(std::list<QString>& accu, Ts&& ts)
{
  accu.insert(accu.end(), ts.begin(), ts.end());
}

}  // namespace

namespace omm
{
void register_everything()
{
  using namespace omm_generated;
  std::list<QString> keys;
  register_managers();
  merge(keys, Manager::keys());
  register_objects();
  merge(keys, Object::keys());
  register_tags();
  merge(keys, Tag::keys());
  register_properties();
  merge(keys, Property::keys());
  register_tools();
  merge(keys, Tool::keys());
  register_nodes();
  merge(keys, nodes::Node::keys());

  // assume that the keys are unique, even for different types.
  // However, keys may have the same name as actions.
  for (auto it = keys.begin(); it != keys.end(); ++it) {
    for (auto it2 = keys.begin(); it2 != it; ++it2) {
      if (*it == *it2) {
        LERROR << "Duplicate key: " << *it;
        LFATAL("Duplicate key");
      }
    }
  }
}

}  // namespace omm
