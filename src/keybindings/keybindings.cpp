#include "keybindings/keybindings.h"
#include <QSettings>
#include "keybindings/commandinterface.h"
#include <QKeyEvent>
#include "common.h"
#include <map>
#include <list>
#include "keybindings/action.h"
#include <memory>
#include <QMenu>
#include <QCoreApplication>
#include <QApplication>
#include "keybindings/menu.h"
#include "logging.h"
#include <QKeySequence>

namespace
{

QKeySequence ks(const std::string& s)
{
  return QKeySequence(QString::fromStdString(s));
}

std::pair<std::string, std::string> split(const std::string& path)
{
  constexpr auto separator = '/';
  const auto it = std::find(path.rbegin(), path.rend(), separator);
  if (it == path.rend()) {
    return { "", path };
  } else {
    const auto i = std::distance(it, path.rend());
    assert(i > 1);
    return { path.substr(0, static_cast<std::size_t>(i-1)),
             path.substr(static_cast<std::size_t>(i)) };
  }
}

std::unique_ptr<QMenu> add_menu(const std::string& path, std::map<std::string, QMenu*>& menu_map)
{
  if (menu_map.count(path) > 0) {
    return nullptr;
  } else {
    const auto [ rest_path, menu_name ] = split(path);
    const auto menu_label = QCoreApplication::translate("menu_name", menu_name.c_str());
    std::unique_ptr<QMenu> menu = std::make_unique<omm::Menu>(menu_label);
    menu->setObjectName(QString::fromStdString(menu_name));
    menu_map.insert({ path, menu.get() });

    if (rest_path.empty()) {
      return menu;  // menu is top-level and did not exist before.
    } else {
      auto top_level_menu = add_menu(rest_path, menu_map);
      assert(menu_map.count(rest_path) > 0);
      menu_map[rest_path]->addMenu(menu.release());
      return top_level_menu;  // may be nullptr if top_level_menu already existed before.
    }
  }
}

}  // namespace

namespace omm
{

KeyBindings::KeyBindings() : PreferencesTree(":/keybindings/default_keybindings.cfg")
{
  load_from_qsettings(keybindings_group);
}

KeyBindings::~KeyBindings()
{
  save_in_qsettings(keybindings_group);
}

std::string KeyBindings::find_action(const std::string& context, const QKeySequence& sequence) const
{
  auto* group = this->group(context);
  if (group == nullptr) {
    return "";
  } else {
    const auto vit = std::find_if(group->values.begin(), group->values.end(), [&sequence](auto&& v) {
      const auto other = QKeySequence(QString::fromStdString(v->value()));
      return other.matches(sequence) == QKeySequence::ExactMatch;
    });
    if (vit == group->values.end()) {
      return "";
    } else {
      return vit->get()->name;
    }
  }
}

QVariant KeyBindings::data(int column, const PreferencesTreeValueItem& item, int role) const
{
  switch (role) {
  case Qt::EditRole:
    return ks(item.value());
  case Qt::BackgroundRole:
    if (collides(item)){
      return QColor(Qt::red).lighter();
    } else {
      return QVariant();
    }
  case Qt::DisplayRole:
    return ks(item.value(column-1)).toString(QKeySequence::NativeText);
  case Qt::FontRole:
    if (ks(item.default_value(column-1)).matches(ks(item.value())) != QKeySequence::ExactMatch) {
      auto font = QApplication::font();
      font.setItalic(true);
      return font;
    } else {
      return QVariant();
    }
  case DEFAULT_VALUE_ROLE:
    return ks(item.default_value(column-1));
  default:
    return QVariant();
  }
}

bool KeyBindings::set_data(int column, PreferencesTreeValueItem& item, const QVariant& value)
{
  const QKeySequence sequence = value.value<QKeySequence>();
  item.set_value(sequence.toString(QKeySequence::PortableText).toStdString(), column-1);
  return true;
}

std::string KeyBindings::translate(const std::string& group, const std::string& text) const
{
  return QCoreApplication::translate(("keybindings/" + group).c_str(), text.c_str()).toStdString();
}

std::pair<std::string, QMenu*>
KeyBindings::get_menu( const std::string& action_path, std::map<std::string, QMenu*>& menu_map,
                       std::list<std::unique_ptr<QMenu>>& menus)
{
  const auto [path, action_name] = split(action_path);
  auto menu = add_menu(path, menu_map);
  assert(menu_map.count(path) > 0);
  QMenu* menu_ptr = nullptr;
  if (menu) {
    menu_ptr = menu.get();
    menus.push_back(std::move(menu));
  } else {
    menu_ptr = menu_map.at(path);
  }
  return std::pair(action_name, menu_ptr);
}

bool KeyBindings::collides(const PreferencesTreeValueItem& candidate) const
{
  const QKeySequence sequence = ks(candidate.value());
  if (sequence.isEmpty()) {
    return false;
  }
  PreferencesTreeGroupItem& group = *this->group(candidate.group);
  const auto it = std::find_if(group.values.begin(), group.values.end(),
                               [sequence, name=candidate.name](const auto& value)
  {
    if (value->name == name) {
      return false;
    }
    const QKeySequence other = ks(value->value());
    if (other.isEmpty()) {
      return false;
    }
    for (int i = 0; i < std::min(other.count(), sequence.count()); ++i) {
      if (other[i] != sequence[i]) {
        return false;
      }
    }
    return true;
  });
  return it != group.values.end();
}

std::unique_ptr<QAction>
KeyBindings::make_action(CommandInterface& context, const std::string& action_name) const
{
  const auto* group = this->group(context.type());

#ifndef NDEBUG
  if (group == nullptr) {
    LERROR << "Failed to find context " << context.type();
    LFATAL("Missing context");
  }
#endif  // NDEBUG

  const auto it = std::find_if(group->values.begin(), group->values.end(),
                               [action_name](const auto& value)
  {
    return action_name == value->name;
  });

#ifndef NDEBUG
  if (it == group->values.end()) {
    LERROR << "Failed to find keybinding for " << context.type() << "::" << action_name;
    LFATAL("Missing keybinding");
  }
#endif  // NDEBUG

  auto action = std::make_unique<Action>(**it);
  QObject::connect(action.get(), &QAction::triggered, [&context, action_name] {
    context.perform_action(action_name);
  });
  return action;
}

std::vector<std::unique_ptr<QMenu>>
KeyBindings::make_menus(CommandInterface& context, const std::vector<std::string>& actions) const
{
  std::list<std::unique_ptr<QMenu>> menus;
  std::map<std::string, QMenu*> menu_map;
  for (const auto& action_path : actions) {
    auto [ action_name, menu ] = get_menu(action_path, menu_map, menus);
    if (action_name == SEPARATOR) {
      menu->addSeparator();
    } else if (!action_name.empty()) {
      menu->addAction(make_action(context, action_name).release());
    }
  }

  return std::vector( std::make_move_iterator(menus.begin()),
                      std::make_move_iterator(menus.end()) );
}

}  // namespace omm
