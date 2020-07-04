#include "registers.h"
#include "logging.h"
#include <list>

#include "register_managers.cpp"
#include "register_objects.cpp"
#include "register_tags.cpp"
#include "register_properties.cpp"
#include "register_tools.cpp"
#include "register_nodes.cpp"

namespace
{

template<typename Ts>
void merge(std::list<QString>& accu, Ts&& ts)
{
  accu.insert(accu.end(), ts.begin(), ts.end());
}

}  // namespace

namespace omm
{

void register_everything()
{
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
  merge(keys, Node::keys());

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
