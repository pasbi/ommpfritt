#include "managers/propertymanager/propertymanager.h"

#include <algorithm>
#include <set>
#include <QTabWidget>

#include "managers/propertymanager/propertymanagertab.h"
#include "propertywidgets/propertywidget.h"
#include "common.h"

namespace
{

using Key = omm::HasProperties::Key;
using SetOfHasProperties = std::set<omm::HasProperties*>;

std::vector<Key> get_key_intersection(const SetOfHasProperties& selection)
{
  if (selection.size() == 0) {
    return std::vector<Key>();
  }

  const auto* the_entity = *selection.begin();
  auto keys = the_entity->properties().keys();
  std::unordered_map<Key, std::string> types;
  types.reserve(keys.size());
  for (auto&& key : keys) {
    types.insert(std::make_pair(key, the_entity->property(key).type()));
  }

  const auto has_key = [](const omm::HasProperties* entity, const Key& key) {
    auto&& property_keys = entity->properties().keys();
    return std::find(property_keys.begin(), property_keys.end(), key) != property_keys.end();
  };

  const auto key_same_type = [types](const omm::HasProperties* entity, const Key& key) {
    return types.at(key) == entity->property(key).type();
  };


  for (auto it = std::next(selection.begin()); it != selection.end(); ++it) {
    const auto not_has_key_of_same_type = [&it, &has_key, &key_same_type](const Key& key) {
      return !has_key(*it, key) || !key_same_type(*it, key);
    };
    keys.erase(std::remove_if(keys.begin(), keys.end(), not_has_key_of_same_type), keys.end());
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

auto collect_properties(const omm::HasProperties::Key& key, const SetOfHasProperties& selection)
{
  std::set<omm::Property*> collection;
  const auto f = [key](omm::HasProperties* entity) {
    return &entity->property(key);
  };

  return transform<omm::Property*>(selection, f);
}

std::string get_tab_label(const omm::Property::SetOfProperties& properties)
{
  assert(properties.size() > 0);
  const auto tab_label = (*properties.begin())->category();
#ifndef NDEBUG
  for (auto&& property : properties) {
    assert(property != nullptr);
    if (tab_label != property->category()) {
      LOG(WARNING) << "category is not consistent: "
                   << "'" << tab_label << "' != '" << property->category() << "'.";
    }
  }
#endif
  return tab_label;
}

}  // namespace

namespace omm
{

PropertyManager::PropertyManager(Scene& scene)
  : Manager(tr("Properties"), scene)
  , m_tabs(transfer( std::make_unique<QTabWidget>(),
                     [this](std::unique_ptr<QTabWidget> tabs) {
                        this->setWidget(tabs.release());
                      } ) )
{
  setWindowTitle(tr("property manager"));
  setObjectName(TYPE());
  m_scene.ObserverRegister<AbstractSelectionObserver>::register_observer(*this);
}

PropertyManager::~PropertyManager()
{
  m_scene.ObserverRegister<AbstractSelectionObserver>::unregister_observer(*this);
}

void PropertyManager::set_selection(const SetOfHasProperties& selection)
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
    tabs.at(tab_label)->add_properties(m_scene, properties);
  }

  for (auto&& tab_label : tabs.keys()) {
    auto& tab = tabs.at(tab_label);
    tab->end_add_properties();
    m_tabs.addTab(tab.release(), QString::fromStdString(tab_label));
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
