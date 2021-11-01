#include "keybindings/keybindings.h"
#include "common.h"
#include "keybindings/commandinterface.h"
#include "logging.h"
#include "main/application.h"
#include "preferences/preferencestreeitem.h"
#include <QApplication>
#include <QCoreApplication>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMenu>
#include <QSettings>
#include <list>
#include <map>
#include <memory>

namespace
{
std::pair<QString, QString> split(const QString& path)
{
  constexpr auto separator = '/';
  const auto it = std::find(path.rbegin(), path.rend(), separator);
  if (it == path.rend()) {
    return {"", path};
  } else {
    const auto i = std::distance(it, path.rend());
    assert(i > 1);
    return {path.mid(0, static_cast<int>(i - 1.0)), path.mid(i)};
  }
}

std::unique_ptr<QMenu> add_menu(const QString& path, std::map<QString, QMenu*>& menu_map)
{
  if (menu_map.count(path) > 0) {
    return nullptr;
  } else {
    const auto [rest_path, menu_name] = split(path);
    const auto menu_label
        = QCoreApplication::translate("menu_name", menu_name.toUtf8().constData());
    auto menu = std::make_unique<QMenu>(menu_label);
    menu->setTearOffEnabled(true);
    menu->setObjectName(menu_name);
    menu_map.insert({path, menu.get()});

    if (rest_path.isEmpty()) {
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
KeyBindings::KeyBindings()
    : PreferencesTree(TRANSLATION_CONTEXT, ":/keybindings/default_keybindings.cfg")
{
  load_from_qsettings(TRANSLATION_CONTEXT);
  std::set<QString> names;
  for (auto&& group : groups()) {
    for (auto&& value : group->values) {
      const auto name = value->name;
      if (::contains(names, name)) {
        LERROR << "Duplicate action name: '" << name << "'.";
        LERROR << "Action names must be unique, even across contextes.";
        LFATAL("Duplicate action name.");
      } else {
        names.insert(name);
      }
    }
  }
}

KeyBindings::~KeyBindings()
{
  save_in_qsettings(TRANSLATION_CONTEXT);
}

std::unique_ptr<QAction> KeyBindings::make_menu_action(CommandInterface& context,
                                                       const QString& action_name) const
{
  return make_action(context, action_name, Target::Menu);
}

std::unique_ptr<QAction> KeyBindings::make_toolbar_action(CommandInterface& context,
                                                          const QString& action_name) const
{
  return make_action(context, action_name, Target::ToolBar);
}

QString KeyBindings::find_action(const QString& context, const QKeySequence& sequence) const
{
  const auto* group = this->group(context);
  if (group == nullptr) {
    return "";
  } else {
    QKeySequence candidate = sequence;
    while (!candidate.isEmpty()) {
      const auto vit
          = std::find_if(group->values.begin(), group->values.end(), [&candidate](auto&& v) {
              const auto other = QKeySequence(v->value());
              return other.matches(candidate) == QKeySequence::ExactMatch;
            });
      if (vit != group->values.end()) {
        return vit->get()->name;
      } else {
        candidate = QKeySequence(candidate[1], candidate[2], candidate[3], 0);
      }
    }
    return "";
  }
}

const PreferencesTreeValueItem* KeyBindings::find_action(const QString& context,
                                                         const QString& action_name) const
{
  const auto* group = this->group(context);
  const auto it
      = std::find_if(group->values.begin(), group->values.end(), [action_name](const auto& value) {
          return action_name == value->name;
        });
  if (it == group->values.end()) {
    return nullptr;
  } else {
    return it->get();
  }
}

QVariant KeyBindings::data(int column, const PreferencesTreeValueItem& item, int role) const
{
  switch (role) {
  case Qt::EditRole:
    return QKeySequence(item.value());
  case Qt::BackgroundRole:
    if (collides(item)) {
      return QColor(Qt::red).lighter();
    } else {
      return QVariant();
    }
  case Qt::DisplayRole:
    return QKeySequence(item.value(column - 1)).toString(QKeySequence::NativeText);
  case Qt::FontRole:
    if (QKeySequence(item.default_value(column - 1)).matches(QKeySequence(item.value()))
        != QKeySequence::ExactMatch) {
      auto font = QApplication::font();
      font.setItalic(true);
      return font;
    } else {
      return QVariant();
    }
  case DEFAULT_VALUE_ROLE:
    return QKeySequence(item.default_value(column - 1));
  default:
    return QVariant();
  }
}

bool KeyBindings::set_data(int column, PreferencesTreeValueItem& item, const QVariant& value)
{
  const auto sequence = value.value<QKeySequence>();
  item.set_value(sequence.toString(QKeySequence::PortableText), column - 1);
  return true;
}

std::pair<QString, QMenu*> KeyBindings::get_menu(const QString& action_path,
                                                 std::map<QString, QMenu*>& menu_map,
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
  const QKeySequence sequence = QKeySequence(candidate.value());
  if (sequence.isEmpty()) {
    return false;
  }
  PreferencesTreeGroupItem& group = *this->group(candidate.group);
  const auto it
      = std::find_if(group.values.begin(),
                     group.values.end(),
                     [sequence, name = candidate.name](const auto& value) {
                       if (value->name == name) {
                         return false;
                       }
                       const QKeySequence other(value->value());
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

std::vector<std::unique_ptr<QMenu>>
KeyBindings::make_menus(CommandInterface& context, const std::vector<QString>& actions) const
{
  std::list<std::unique_ptr<QMenu>> menus;
  std::map<QString, QMenu*> menu_map;
  for (const auto& action_path : actions) {
    auto [action_name, menu] = get_menu(action_path, menu_map, menus);
    if (action_name == SEPARATOR) {
      menu->addSeparator();
    } else if (!action_name.isEmpty()) {
      menu->addAction(make_menu_action(context, action_name).release());
    }
  }

  return std::vector(std::make_move_iterator(menus.begin()), std::make_move_iterator(menus.end()));
}

std::unique_ptr<QAction>
KeyBindings::make_action(CommandInterface& context, const QString& action_name, Target target) const
{
  const auto* item = find_action(context.type(), action_name);
#ifndef NDEBUG
  if (item == nullptr) {
    LERROR << "Failed to find keybinding for " << context.type() << "::" << action_name;
    LFATAL("Missing keybinding");
  }
#endif  // NDEBUG

  auto action = std::make_unique<QAction>();
  action->setIcon(item->icon());
  const auto label = item->translated_name();
  action->setText(label);
  action->setToolTip(label);
  if (target == Target::Menu) {
    using namespace omm;
    QObject::connect(
        item,
        &PreferencesTreeValueItem::value_changed,
        action.get(),
        [action = action.get()](const QString& s) { action->setShortcut(QKeySequence(s)); });
    action->setShortcut(QKeySequence(item->value()));
  }
  QObject::connect(action.get(), &QAction::triggered, [&context, action_name] {
    context.perform_action(action_name);
  });
  return action;
}

}  // namespace omm
