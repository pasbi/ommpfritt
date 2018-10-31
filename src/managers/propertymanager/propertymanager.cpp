#include "managers/propertymanager/propertymanager.h"

#include <algorithm>
#include <set>
#include <QTabWidget>

#include "managers/propertymanager/propertymanagertab.h"

namespace
{

using Key = omm::HasProperties::Key;

std::vector<Key>
get_key_intersection(const std::unordered_set<omm::HasProperties*>& selection)
{
  if (selection.size() == 0) {
    return std::vector<Key>();
  }

  using selection_iterator = typename std::decay<decltype(selection)>::type::const_iterator;
  const auto has_key = [](const selection_iterator& it, const Key& key) {
    auto&& property_keys = (*it)->property_keys();
    return std::find(property_keys.begin(), property_keys.end(), key) != property_keys.end();
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

  // select properties with same name and type
  auto keys = (*selection.begin())->property_keys();
  for (selection_iterator it = std::next(selection.begin()); it != selection.end(); ++it)
  {
    const auto predicate = [&it, has_key_of_same_type](const Key& key) {
      return !has_key_of_same_type(it, key);
    };
    keys.erase(std::remove_if(keys.begin(), keys.end(), predicate), keys.end());
  }

  return keys;
}

void split_key(const std::string& key, std::string& tab_name, std::string& property_name)
{
  constexpr auto character = '/';
  size_t pos = key.find(character);
  if (pos == std::string::npos) {
    tab_name = "";
    property_name = key;
  } else {
    tab_name = key.substr(0, pos);
    property_name = key.substr(pos + 1);
  }
}

std::vector<omm::Property*>
collect_properties( const omm::HasProperties::Key& key,
                    const std::unordered_set<omm::HasProperties*>& selection )
{
  std::vector<omm::Property*> collection;
  collection.reserve(selection.size());
  const auto f = [key](omm::HasProperties* entity) {
    return &entity->property(key);
  };
  std::transform(selection.begin(), selection.end(), std::back_inserter(collection), f);
  return collection;
}

template<typename T, typename F> T& transfer(std::unique_ptr<T> object, F consumer)
{
  T& ref = *object;
  consumer(std::move(object));
  return ref;
}

std::string get_tab_label(const std::vector<omm::Property*>& properties)
{
  assert(properties.size() > 0);
  const auto tab_label = properties.front()->category();
#ifndef NDEBUG
  for (auto&& property : properties) {
    assert(property != nullptr);
    assert(tab_label == property->category());
  }
#endif
  return tab_label;
}

}  // namespace

namespace omm
{

PropertyManager::PropertyManager(Scene& scene)
  : Manager(tr("Properties"), scene)
  , m_tabs(transfer(std::make_unique<QTabWidget>(), [this](std::unique_ptr<QTabWidget> tabs) {
    this->setWidget(tabs.release());
  }))
{
  setWindowTitle(tr("property manager"));
  setObjectName(TYPE());
  m_scene.ObserverRegister<AbstractPropertyObserver>::register_observer(*this);
}

PropertyManager::~PropertyManager()
{
  m_scene.ObserverRegister<AbstractPropertyObserver>::unregister_observer(*this);
}

void PropertyManager::set_selection(const std::unordered_set<HasProperties*>& selection)
{
  const auto key_intersection = get_key_intersection(selection);
  clear();
  OrderedMap<std::string, std::unique_ptr<PropertyManagerTab>> tabs;

  for (const auto& key : key_intersection) {
    const auto properties = collect_properties(key, selection);
    assert(properties.size() > 0);
    const auto tab_label = get_tab_label(properties);
    if (!tabs.contains(tab_label)) {
      tabs.insert(tab_label, std::make_unique<PropertyManagerTab>());
    }
    tabs.at(tab_label)->add_properties(properties);
  }

  for (auto&& tab_label : tabs) {
    m_tabs.addTab(tabs.at(tab_label).release(), QString::fromStdString(tab_label));
  }
}

void PropertyManager::clear()
{
  for (int i = 0; i < m_tabs.count(); ++i) {
    delete m_tabs.widget(i);
  }
  m_tabs.clear();
}

}  // namespace omm
