#include "managers/propertymanager.h"

#include <algorithm>
#include <set>

namespace {
  using KeyTypeMap = std::map<omm::HasProperties::PropertyKey, std::type_index>;
  KeyTypeMap get_keys_and_types(const omm::HasProperties& item)
  {
    KeyTypeMap keys_and_types;
    for ( const std::string& key : item.property_keys() )
    {
      keys_and_types.insert(std::make_pair(key, item.property(key).type_index()));
    }
    return keys_and_types;
  }

  void intersect_keys_and_types(KeyTypeMap& a, const KeyTypeMap& b)
  {
    auto keep_item = [](const KeyTypeMap::value_type& item, const KeyTypeMap& map){
      return map.count(item.first) > 0 && map.at(item.first) == item.second;
    };

    for (auto it = a.cbegin(); it != a.cend();)
    {
      if (keep_item(*it, b)) {
        ++it;
      } else {
        it = a.erase(it);
      }
    }
  }

}  // namespace

namespace omm
{

PropertyManager::PropertyManager(Scene& scene)
  : Manager(tr("Properties"), scene)
{
  setWindowTitle(tr("property manager"));
  setObjectName(TYPE());
}

using HasPropertiesRef = std::reference_wrapper<omm::HasProperties>;
void omm::PropertyManager::select( const std::vector<HasPropertiesRef>& selection)
{
  // select properties with same name and type

  KeyTypeMap keyTypeMap;
  if (selection.size() > 0) {
    keyTypeMap = get_keys_and_types(selection.at(0).get());
  }
  for (HasProperties& item : selection) {
    intersect_keys_and_types(keyTypeMap, get_keys_and_types(item));
  }

}

}  // namespace omm
