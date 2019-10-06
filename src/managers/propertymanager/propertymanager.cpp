#include "managers/propertymanager/propertymanager.h"

#include <algorithm>
#include <set>
#include <QTimer>
#include <QCoreApplication>
#include <QPushButton>
#include <QTabWidget>

#include "scene/messagebox.h"
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
  std::map<omm::AbstractPropertyOwner*, omm::Property*> collection;
  for (omm::AbstractPropertyOwner* owner : selection) {
    collection.insert(std::pair(owner, owner->property(key)));
  }
  return collection;
}

std::string get_tab_label(const std::map<omm::AbstractPropertyOwner*, omm::Property*>& properties)
{
  assert(properties.size() > 0);
  const auto tab_label = (*properties.begin()).second->category();
#ifndef NDEBUG
  for (auto&& [_, property ] : properties) {
    Q_UNUSED(_)
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
  : Manager(QCoreApplication::translate("any-context", "Properties"), scene)
{
  auto main_layout = std::make_unique<QVBoxLayout>();

  m_tab_bar = std::make_unique<MultiTabBar>();
  main_layout->addWidget(m_tab_bar.get());

  m_scroll_area = std::make_unique<QScrollArea>();
  m_scroll_area->setWidgetResizable(true);
  auto category_widget = std::make_unique<QWidget>();
  auto layout = std::make_unique<QVBoxLayout>();
  m_layout = layout.get();
  category_widget->setLayout(layout.release());
  m_layout->setContentsMargins(0, 0, 6, 0);
  m_scroll_area->setWidget(category_widget.release());
  main_layout->addWidget(m_scroll_area.get());

  auto central_widget = std::make_unique<QWidget>();
  central_widget->setLayout(main_layout.release());
  set_widget(std::move(central_widget));

  setWindowTitle(QString::fromStdString(make_window_title()));
  setObjectName(TYPE);

  connect(m_tab_bar.get(), SIGNAL(current_indices_changed(const std::set<int>&)),
          this, SLOT(activate_tabs(const std::set<int>&)));
  connect(&scene.message_box(), SIGNAL(selection_changed(std::set<AbstractPropertyOwner*>)),
          this, SLOT(set_selection(std::set<AbstractPropertyOwner*>)));
  m_scroll_area->setFrameShape(QFrame::NoFrame);
}

PropertyManager::~PropertyManager()
{
  clear();
}

std::unique_ptr<QWidget> PropertyManager::make_menu_bar()
{
//  auto menu_bar = std::make_unique<QMenuBar>();
//  auto user_properties_menu = menu_bar->addMenu(QObject::tr("user properties", "PropertyManager"));
//  const auto exec_user_property_dialog = [this]() {
//    auto dialog = UserPropertyDialog(**m_current_selection.begin(), this);
//    dialog.exec();
//  };
//  m_manage_user_properties_action = &action( *user_properties_menu,
//                                             QObject::tr("edit ...", "PropertyManager"),
//                                             exec_user_property_dialog );
//  m_manage_user_properties_action->setEnabled(false);

//  auto lock_button = std::make_unique<QPushButton>();
//  lock_button->setFixedSize(24, 24);
//  lock_button->setText("L");
//  lock_button->setCheckable(true);
//  connect(lock_button.get(), &QPushButton::toggled, [this](bool checked) { set_locked(checked); });

//  auto container = std::make_unique<QWidget>();
//  auto layout = std::make_unique<QHBoxLayout>();
//  layout->addWidget(menu_bar.release());
//  layout->addWidget(lock_button.release());
//  container->setLayout(layout.release());
//  return container;
  return nullptr;
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
  {
    QSignalBlocker blocker(m_tab_bar.get());
    for (auto&& tab_label : m_tabs.keys()) {
      auto& tab = m_tabs.at(tab_label);
      const QString display_name = tab_display_name(tab_label);
      tab_display_names.insert(display_name);
      m_tab_bar->add_tab(display_name);
      m_layout->addWidget(tab.get());
    }
  }

  {
    const auto it = m_current_categroy_indices.find(m_current_selection);
    if (it == m_current_categroy_indices.cend()) {
      activate_tabs({0});
    } else {
      activate_tabs(it->second);
    }
  }

  m_layout->addStretch();
  setWindowTitle(QString::fromStdString(make_window_title()));
}

void PropertyManager::set_locked(bool locked) { m_is_locked = locked; }

void PropertyManager::clear()
{
  for (QWidget* widget : m_tabs.values()) {
    m_layout->removeWidget(widget);
  }
  m_tabs.clear();

  for (int i = m_layout->count() - 1; i >= 0; --i) {
    m_layout->removeItem(m_layout->itemAt(i));
  }

  {
    QSignalBlocker blocker(m_tab_bar.get());
    m_tab_bar->clear();
  }
}

std::string PropertyManager::type() const { return TYPE; }

bool PropertyManager::perform_action(const std::string& name)
{
  LINFO << name;
  return false;
}

std::string PropertyManager::make_window_title() const
{
  std::ostringstream ss;
  ss << QObject::tr("property manager", "PropertyManager").toStdString();
  for (auto&& selected : m_current_selection) {
    ss << " " << selected->name();
  }
  return ss.str();
}

void PropertyManager::activate_tabs(const std::set<int>& indices)
{
  QSignalBlocker blocker(m_tab_bar.get());
  m_tab_bar->set_current_indices(indices);
  const std::vector<PropertyManagerTab*> tabs = m_tabs.values();
  for (PropertyManagerTab* w : tabs) {
    w->hide();
  }
  if (!indices.empty() && !tabs.empty()) {
    m_current_categroy_indices[m_current_selection] = indices;
    for (int index : indices) {
      tabs[index]->show();
    }
  }
}

}  // namespace omm
