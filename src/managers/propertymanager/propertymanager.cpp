#include "managers/propertymanager/propertymanager.h"

#include <QCoreApplication>
#include <QPushButton>
#include <QTabWidget>
#include <QTimer>
#include <algorithm>
#include <set>

#include "aspects/propertyowner.h"
#include "common.h"
#include "logging.h"
#include "mainwindow/iconprovider.h"
#include "managers/propertymanager/propertymanagertab.h"
#include "managers/propertymanager/propertymanagertitlebar.h"
#include "properties/optionproperty.h"
#include "propertywidgets/propertywidget.h"
#include "scene/mailbox.h"
#include "scene/scene.h"

namespace
{
QString tab_display_name(const QString& tab_name)
{
  if (tab_name == omm::Property::USER_PROPERTY_CATEGROY_NAME) {
    return QObject::tr(tab_name.toUtf8().constData());
  } else {
    return tab_name;
  }
}

std::vector<QString> get_key_intersection(const std::set<omm::AbstractPropertyOwner*>& selection)
{
  if (selection.empty()) {
    return std::vector<QString>();
  }

  const auto* the_entity = *selection.begin();
  auto keys = the_entity->properties().keys();
  std::map<QString, omm::Property*> the_properties;
  for (auto&& key : keys) {
    the_properties.insert({key, the_entity->property(key)});
  }

  for (auto it = std::next(selection.begin()); it != selection.end(); ++it) {
    const auto has_key_of_same_type = [&](const QString& key) {
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

auto collect_properties(const QString& key, const std::set<omm::AbstractPropertyOwner*>& selection)
{
  std::map<omm::AbstractPropertyOwner*, omm::Property*> collection;
  for (omm::AbstractPropertyOwner* owner : selection) {
    collection.insert(std::pair(owner, owner->property(key)));
  }
  return collection;
}

QString get_tab_label(const std::map<omm::AbstractPropertyOwner*, omm::Property*>& properties)
{
  assert(!properties.empty());
  auto tab_label = (*properties.begin()).second->category();
#ifndef NDEBUG
  for (auto&& [_, property] : properties) {
    Q_UNUSED(_)
    assert(property != nullptr);
    if (tab_label != property->category()) {
      LWARNING << "category is not consistent: '" << tab_label << "' != '" << property->category()
               << "'.";
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
  auto title_bar = std::make_unique<PropertyManagerTitleBar>(*this);
  m_title_bar = title_bar.get();
  setTitleBarWidget(title_bar.release());

  m_tab_bar = std::make_unique<MultiTabBar>();

  m_scroll_area = std::make_unique<QScrollArea>();
  m_scroll_area->setWidgetResizable(true);

  auto category_widget = std::make_unique<QWidget>();
  auto layout = std::make_unique<QVBoxLayout>();
  m_layout = layout.get();
  category_widget->setLayout(layout.release());
  static constexpr int TOP_MARGIN = 6;
  m_layout->setContentsMargins(0, 0, TOP_MARGIN, 0);
  m_scroll_area->setWidget(category_widget.release());

  auto main_layout = std::make_unique<QVBoxLayout>();

  auto hlayout = std::make_unique<QHBoxLayout>();
  m_icon_label = std::make_unique<QLabel>("");
  m_selection_label = std::make_unique<QLabel>("");
  hlayout->addWidget(m_icon_label.get(), 0);
  hlayout->addWidget(m_selection_label.get(), 1);
  main_layout->addLayout(hlayout.release());

  main_layout->addWidget(m_tab_bar.get());
  main_layout->addWidget(m_scroll_area.get());

  auto central_widget = std::make_unique<QWidget>();
  central_widget->setLayout(main_layout.release());
  set_widget(std::move(central_widget));

  setWindowTitle(make_window_title());

  connect(m_tab_bar.get(),
          &MultiTabBar::current_indices_changed,
          this,
          &PropertyManager::activate_tabs);
  connect(&scene.mail_box(), &MailBox::selection_changed, this, &PropertyManager::set_selection);
  m_scroll_area->setFrameShape(QFrame::NoFrame);
}

PropertyManager::~PropertyManager()
{
  clear();
}

void PropertyManager::set_selection(const std::set<AbstractPropertyOwner*>& selection)
{
  if (!is_locked()) {
    for (auto* apo : m_current_selection) {
      disconnect(apo,
                 &AbstractPropertyOwner::property_visibility_changed,
                 this,
                 &PropertyManager::update_property_widgets);
    }

    m_current_selection = selection;
    for (auto* apo : m_current_selection) {
      connect(apo,
              &AbstractPropertyOwner::property_visibility_changed,
              this,
              &PropertyManager::update_property_widgets,
              Qt::QueuedConnection);
    }

    update_property_widgets();
    m_title_bar->set_selection(selection);

    m_icon_label->setVisible(!selection.empty());
    m_selection_label->setVisible(!selection.empty());
    if (!selection.empty()) {
      const auto types = util::transform(selection, [](AbstractPropertyOwner* owner) {
        return owner->type();
      });
      std::list<QString> tokens;

      if (selection.size() > 1) {
        tokens.push_back(tr("[%n Elements]", "PropertyManager", selection.size()));
      }
      tokens.push_back(types.size() == 1 ? *types.begin() : "");
      const auto names
          = util::transform<std::list>(selection, [](AbstractPropertyOwner* owner) {
              return owner->name();
            });
      static constexpr auto s2s = [](const QString& s) { return s; };
      tokens.push_back("[" + join(names, s2s) + "]");
      m_selection_label->setText(join(tokens, s2s));

      // text in m_selection_label can get huge but is not very important. Don't mess up the layout.
      m_selection_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
      const auto icon_name = types.size() == 1 ? *types.begin() : "undetermined-type";
      const auto image = IconProvider::pixmap(icon_name);
      if (image.isNull()) {
        m_icon_label->clear();
      } else {
        static constexpr QSize ICON_SIZE{24, 24};
        const auto scaled = image.scaled(ICON_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_icon_label->setPixmap(scaled);
      }
    }
  }
}

void PropertyManager::update_property_widgets()
{
  clear();
  for (const auto& key : get_key_intersection(m_current_selection)) {
    const auto properties = collect_properties(key, m_current_selection);
    assert(!properties.empty());
    const auto tab_label = get_tab_label(properties);
    if (!m_tabs.contains(tab_label)) {
      m_tabs.insert(tab_label, std::make_unique<PropertyManagerTab>(tab_label));
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
      activate_tabs({});
    } else {
      activate_tabs(it->second);
    }
  }

  m_layout->addStretch(1);
  setWindowTitle(make_window_title());
}

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

QString PropertyManager::type() const
{
  return TYPE;
}

bool PropertyManager::perform_action(const QString& name)
{
  LINFO << name;
  return false;
}

QString PropertyManager::make_window_title() const
{
  QString title = QObject::tr("property manager", "PropertyManager");
  for (auto&& selected : m_current_selection) {
    title += " " + selected->name();
  }
  return title;
}

void PropertyManager::activate_tabs(const std::set<int>& indices)
{
  QSignalBlocker blocker(m_tab_bar.get());
  m_tab_bar->set_current_indices(indices);
  const std::vector<PropertyManagerTab*> tabs = m_tabs.values();
  for (PropertyManagerTab* w : tabs) {
    w->hide();
  }

  const bool header_visible = indices.size() != 1 && m_tabs.size() > 1;
  if (!indices.empty() && !tabs.empty()) {
    m_current_categroy_indices[m_current_selection] = indices;
    for (int index : indices) {
      if (static_cast<std::size_t>(index) < tabs.size()) {
        tabs[index]->show();
        tabs[index]->set_header_visible(header_visible);
      }
    }
  } else if (indices.empty()) {
    for (auto&& tab : tabs) {
      tab->show();
      tab->set_header_visible(header_visible);
    }
  }
}

}  // namespace omm
