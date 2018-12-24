#include "managers/propertymanager/propertymanager.h"

#include <algorithm>
#include <set>
#include <QTabWidget>

#include "managers/propertymanager/propertymanagertab.h"
#include "propertywidgets/propertywidget.h"
#include "aspects/propertyowner.h"
#include "common.h"

namespace
{

using Key = omm::AbstractPropertyOwner::Key;

std::vector<Key> get_key_intersection(const std::set<omm::AbstractPropertyOwner*>& selection)
{
  if (selection.size() == 0) {
    return std::vector<Key>();
  }

  const auto* the_entity = *selection.begin();
  auto keys = the_entity->properties().keys();
  std::unordered_map<Key, omm::Property*> the_properties;
  for (auto&& key : keys) {
    the_properties.insert(std::make_pair(key, &the_entity->property(key)));
  }

  const auto has_key = [](const omm::AbstractPropertyOwner* entity, const Key& key) {
    auto&& property_keys = entity->properties().keys();
    return std::find(property_keys.begin(), property_keys.end(), key) != property_keys.end();
  };

  const auto
  key_same_type = [the_properties](const omm::AbstractPropertyOwner* entity, const Key& key) {
    return the_properties.at(key)->is_compatible(entity->property(key));
  };

  for (auto it = std::next(selection.begin()); it != selection.end(); ++it) {
    const auto not_has_key_of_same_type = [&it, &has_key, &key_same_type](const Key& key) {
      return !has_key(*it, key) || !key_same_type(*it, key);
    };
    keys.erase(std::remove_if(keys.begin(), keys.end(), not_has_key_of_same_type), keys.end());
  }

  return keys;
}

auto collect_properties( const omm::AbstractPropertyOwner::Key& key,
                         const std::set<omm::AbstractPropertyOwner*>& selection )
{
  std::set<omm::AbstractPropertyOwner*> collection;
  const auto f = [key](omm::AbstractPropertyOwner* entity) {
    return &entity->property(key);
  };

  return transform<omm::Property*>(selection, f);
}

std::string get_tab_label(const std::set<omm::Property*>& properties)
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

size_t find_tab_label(const std::string& label, const std::vector<std::string>& labels)
{
  const auto it = std::find(labels.cbegin(), labels.cend(), label);
  assert(it != labels.cend());
  return std::distance(labels.cbegin(), it);
}

}  // namespace

namespace omm
{

PropertyManager::PropertyManager(Scene& scene)
  : Manager(tr("Properties"), scene)
{
  auto tabs = std::make_unique<QTabWidget>();
  m_tabs = tabs.get();
  set_widget(std::move(tabs));
  setWindowTitle(tr("property manager"));
  setObjectName(TYPE());
  connect(m_tabs, &QTabWidget::currentChanged, [this](int index) {
    if (index >= 0) {
      m_active_category = m_tabs->tabText(index).toStdString();
    }
  });
  scene.Observed<AbstractSelectionObserver>::register_observer(*this);
}

PropertyManager::~PropertyManager()
{
  scene().Observed<AbstractSelectionObserver>::unregister_observer(*this);
}

void PropertyManager::set_selection(const std::set<AbstractPropertyOwner*>& selection)
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

  const auto active_category = m_active_category;
  for (auto&& tab_label : tabs.keys()) {
    auto& tab = tabs.at(tab_label);
    tab->end_add_properties();
    m_tabs->addTab(tab.release(), QString::fromStdString(tab_label));
  }

  if (tabs.contains(active_category)) {
    m_tabs->setCurrentIndex(find_tab_label(active_category, tabs.keys()));
  }
}

void PropertyManager::clear()
{
  const auto active_category = m_active_category;
  while (m_tabs->count() > 0) {
    delete m_tabs->widget(0);
  }
  m_active_category = active_category;
}

}  // namespace omm
