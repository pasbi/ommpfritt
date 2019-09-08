#include "managers/propertymanager/propertymanager.h"

#include <algorithm>
#include <set>
#include <QTimer>
#include <QCoreApplication>
#include <QPushButton>

#include <QTabWidget>
#include <QTabBar>
#include "properties/optionsproperty.h"
#include "managers/propertymanager/propertymanagertab.h"
#include "propertywidgets/propertywidget.h"
#include "aspects/propertyowner.h"
#include "common.h"
#include "menuhelper.h"
#include "logging.h"

namespace
{

QString tab_display_name(const std::string& tab_name) {
  if (tab_name == omm::Property::USER_PROPERTY_CATEGROY_NAME) {
    return QObject::tr(tab_name.c_str());
  } else {
    return QString::fromStdString(tab_name);
  }
}

std::vector<std::string>
get_key_intersection(const std::set<omm::AbstractPropertyOwner*>& selection)
{
  if (selection.size() == 0) {
    return std::vector<std::string>();
  }

  const auto* the_entity = *selection.begin();
  auto keys = the_entity->properties().keys();
  std::unordered_map<std::string, omm::Property*> the_properties;
  for (auto&& key : keys) {
    the_properties.insert(std::make_pair(key, the_entity->property(key)));
  }

  for (auto it = std::next(selection.begin()); it != selection.end(); ++it) {
    const auto has_key_of_same_type = [&](const std::string& key) {
      auto&& property_keys = (*it)->properties().keys();
      if (std::find(property_keys.begin(), property_keys.end(), key) == property_keys.end()) {
        return false;
      } else {
        return the_properties.at(key)->is_compatible(*(*it)->property(key));
      }
    };

    const auto sr = std::remove_if(keys.begin(), keys.end(), std::not_fn(has_key_of_same_type));
    keys.erase(sr, keys.end());
  }

  return keys;
}

auto collect_properties( const std::string& key,
                         const std::set<omm::AbstractPropertyOwner*>& selection )
{
  std::set<omm::AbstractPropertyOwner*> collection;
  const auto f = [key](omm::AbstractPropertyOwner* entity) {
    return entity->property(key);
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
      LWARNING << "category is not consistent: '" << tab_label
                   << "' != '" << property->category() << "'.";
    }
  }
#endif
  return tab_label;
}

}  // namespace

namespace omm
{

PropertyManager::PropertyManager(Scene& scene)
  : Manager(QCoreApplication::translate("any-context", "Properties"), scene, make_menu_bar())
{
  auto main_layout = std::make_unique<QVBoxLayout>();

  m_tab_bar = std::make_unique<QTabBar>();
  m_tab_bar->setAcceptDrops(true);
  m_tab_bar->setChangeCurrentOnDrag(true);
  main_layout->addWidget(m_tab_bar.get());

  m_layout = std::make_unique<QVBoxLayout>();
  main_layout->addLayout(m_layout.get());

  auto central_widget = std::make_unique<QWidget>();
  central_widget->setLayout(main_layout.release());
  set_widget(std::move(central_widget));

  setWindowTitle(QString::fromStdString(make_window_title()));
  setObjectName(TYPE);
  connect(m_tab_bar.get(), SIGNAL(currentChanged(int)), this, SLOT(activate_tab(int)));

  connect(&scene.message_box, SIGNAL(selection_changed(std::set<AbstractPropertyOwner*>)),
          this, SLOT(set_selection(std::set<AbstractPropertyOwner*>)));
}

PropertyManager::~PropertyManager()
{
  clear();
}

std::unique_ptr<QWidget> PropertyManager::make_menu_bar()
{
  auto menu_bar = std::make_unique<QMenuBar>();
  auto user_properties_menu = menu_bar->addMenu(QObject::tr("user properties", "PropertyManager"));
  const auto exec_user_property_dialog = [this]() {
    auto dialog = UserPropertyDialog(this, **m_current_selection.begin());
    if (dialog.exec() == QDialog::Accepted) {
      m_scene.submit(dialog.make_user_property_config_command());
      m_scene.set_selection(m_scene.selection());
    }
  };
  m_manage_user_properties_action = &action( *user_properties_menu,
                                             QObject::tr("edit ...", "PropertyManager"),
                                             exec_user_property_dialog );
  m_manage_user_properties_action->setEnabled(false);

  auto lock_button = std::make_unique<QPushButton>();
  lock_button->setFixedSize(24, 24);
  lock_button->setText("L");
  lock_button->setCheckable(true);
  connect(lock_button.get(), &QPushButton::toggled, [this](bool checked) { set_locked(checked); });

  auto container = std::make_unique<QWidget>();
  auto layout = std::make_unique<QHBoxLayout>();
  layout->addWidget(menu_bar.release());
  layout->addWidget(lock_button.release());
  container->setLayout(layout.release());
  return container;
}

void PropertyManager::set_selection(const std::set<AbstractPropertyOwner*>& selection)
{
  if (!m_is_locked) {
    m_current_selection = selection;
    update_property_widgets();
  }
}

void PropertyManager::update_property_widgets()
{
  clear();
  for (const auto& key : get_key_intersection(m_current_selection)) {
    const auto properties = collect_properties(key, m_current_selection);
    assert(properties.size() > 0);
    const auto tab_label = get_tab_label(properties);
    if (!m_tabs.contains(tab_label)) {
      m_tabs.insert(tab_label, std::make_unique<PropertyManagerTab>());
    }

    m_tabs.at(tab_label)->add_properties(m_scene, key, properties);
  }

  std::set<QString> tab_display_names;
  const auto active_category = m_active_category;
  {
    QSignalBlocker blocker(m_tab_bar.get());
    for (auto&& tab_label : m_tabs.keys()) {
      auto& tab = m_tabs.at(tab_label);
      const QString display_name = tab_display_name(tab_label);
      tab_display_names.insert(display_name);
      m_tab_bar->addTab(display_name);
      m_layout->addWidget(tab.get());
    }
  }

  {
    const auto it = std::find(tab_display_names.cbegin(),
                              tab_display_names.cend(),
                              QString::fromStdString(active_category));
    if (it == tab_display_names.cend()) {
      activate_tab(0);
    } else {
      const int i = std::distance(tab_display_names.cbegin(), it);
      activate_tab(i);
    }
  }

  m_layout->addStretch();

  m_manage_user_properties_action->setEnabled(m_current_selection.size() == 1);
  setWindowTitle(QString::fromStdString(make_window_title()));
}

void PropertyManager::set_locked(bool locked) { m_is_locked = locked; }

void PropertyManager::clear()
{
  const auto active_category = m_active_category;
  for (QWidget* widget : m_tabs.values()) {
    m_layout->removeWidget(widget);
  }
  m_tabs.clear();

  for (int i = m_layout->count() - 1; i >= 0; --i) {
    m_layout->removeItem(m_layout->itemAt(i));
  }

  {
    QSignalBlocker blocker(m_tab_bar.get());
    for (int i = m_tab_bar->count() - 1; i >= 0; --i) {
      m_tab_bar->removeTab(i);
    }
  }

  m_active_category = active_category;
}

std::string PropertyManager::type() const { return TYPE; }

std::string PropertyManager::make_window_title() const
{
  std::ostringstream ss;
  ss << QObject::tr("property manager", "PropertyManager").toStdString();
  for (auto&& selected : m_current_selection) {
    ss << " " << selected->name();
  }
  return ss.str();
}

void PropertyManager::activate_tab(int index)
{
  QSignalBlocker blocker(m_tab_bar.get());
  m_tab_bar->setCurrentIndex(index);
  const std::vector<PropertyManagerTab*> tabs = m_tabs.values();
  for (PropertyManagerTab* w : tabs) {
    w->hide();
  }
  if (index >= 0 && !tabs.empty()) {
    m_active_category = m_tab_bar->tabText(index).toStdString();
    tabs[index]->show();
  }
}

}  // namespace omm
