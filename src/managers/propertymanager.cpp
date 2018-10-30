#include "managers/propertymanager.h"

#include <algorithm>
#include <set>

namespace
{

using Key = omm::HasProperties::Key;

std::unordered_set<Key>
get_key_intersection(const std::unordered_set<omm::HasProperties*>& selection)
{
  if (selection.size() == 0) {
    return std::unordered_set<Key>();
  }

  using selection_iterator = typename std::decay<decltype(selection)>::type::const_iterator;
  const auto has_key = [](const selection_iterator& it, const Key& key) {
    return (*it)->property_keys().count(key) == 1;  // TODO optimize
  };

  const auto get_type = [](const selection_iterator& it, const Key& key) {
    return (*it)->property(key).type_index();
  };

  const auto has_key_of_same_type = [has_key, get_type]( const selection_iterator& it,
                                                         const Key& key )
  {
    if (has_key(it, key)) {
      return get_type(it, key) == get_type(std::prev(it), key);  // same type?
    } else {
      return false;  // not even that key.
    }
  };

  static const auto erase_if_not = [](auto& container, auto predicate) {
    for (auto container_it = container.begin(); container_it != container.end(); ) {
      if (predicate(*container_it)) {
        ++container_it;
      } else {
        container_it = container.erase(container_it);
      }
    }
  };

  // select properties with same name and type
  auto keys = (*selection.begin())->property_keys();
  for (selection_iterator it = std::next(selection.begin()); it != selection.end(); ++it)
  {
    const auto predicate = [&it, has_key_of_same_type](const Key& key) {
      return has_key_of_same_type(it, key);
    };
    erase_if_not(keys, predicate);
  }

  return keys;
}

}  // namespace

namespace omm
{

PropertyManager::PropertyManager(Scene& scene)
  : Manager(tr("Properties"), scene)
{
  setWindowTitle(tr("property manager"));
  setObjectName(TYPE());
  m_scene.AdapterRegister<AbstractPropertyAdapter>::register_adapter(*this);
}

PropertyManager::~PropertyManager()
{
  m_scene.AdapterRegister<AbstractPropertyAdapter>::unregister_adapter(*this);
}

void PropertyManager::set_selection(const std::unordered_set<HasProperties*>& selection)
{
  const auto key_intersection = get_key_intersection(selection);
  LOG(INFO) << "set_selection " << key_intersection.size();
}

}  // namespace omm
