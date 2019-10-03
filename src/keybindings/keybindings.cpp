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
#include "mainwindow/application.h"
#include "managers/stylemanager/stylemanager.h"
#include "managers/objectmanager/objectmanager.h"
#include "managers/pythonconsole/pythonconsole.h"
#include <QCoreApplication>
#include <QApplication>
#include "managers/boundingboxmanager/boundingboxmanager.h"
#include "managers/timeline/timeline.h"
#include <QFile>
#include <QTextStream>
#include <QRegExp>

namespace
{

auto load_default_keybindings(const std::string& filename)
{
  std::vector<omm::ContextKeyBindings> keybindings;
  QFile file(QString::fromStdString(filename));
  if (!file.open(QIODevice::ReadOnly)) {
    LERROR << "Failed to open file '" << filename << "'.";
    return keybindings;
  }

  static const QRegExp context_regexp("\\[\\w+\\]");
  std::string context = "";

  QTextStream stream(&file);
  while (!stream.atEnd()) {
    QString line = stream.readLine();
    line = line.trimmed();
    if (line.startsWith("#") || line.isEmpty()) {
      continue;  // comment
    }

    if (context_regexp.exactMatch(line)) {
      context = line.mid(1, line.size() - 2).toStdString();
    } else if (!context.empty()) {
      const auto tokens = line.split(":");
      if (tokens.size() != 2) {
        LWARNING << "ignoring line '" << line.toStdString()
                     << "'. Expected format: <name>: <key sequence>.";
        continue;
      }
      const auto name = tokens[0].trimmed().toStdString();
      const auto code = tokens[1].trimmed();
      const auto sequence = QKeySequence(code);
      if (sequence.isEmpty() != code.isEmpty()) {
        LWARNING << "Failed to parse key sequence for '" << name
                     << "': '" << code.toStdString() << "'.";
      } else {
        auto it = std::find_if(keybindings.begin(), keybindings.end(),
                                     [context](const omm::ContextKeyBindings& c)
        {
          return c.name == context;
        });

        omm::ContextKeyBindings* c = nullptr;
        if (it == keybindings.end()) {
          keybindings.push_back(omm::ContextKeyBindings(context));
          c = &keybindings.back();
        } else {
          c = &*it;
        }

        const omm::KeyBinding key_binding(name, context, sequence);
        const auto cit = std::find_if(c->key_bindings.begin(), c->key_bindings.end(),
                                      [name](const omm::KeyBinding& k)
        {
          return k.name == name;
        });

        if (cit != c->key_bindings.end()) {
          LWARNING << "Duplicate key sequence for '" << context << "'::'" << name << "'."
                   << "Drop '" << key_binding.key_sequence() << "', "
                   << "keep '" << cit->key_sequence() << "'.";
        } else {
          c->key_bindings.push_back(key_binding);
        }
      }
    } else {
      LWARNING << "line '" << line << "' ignored since no group is active.";
    }

  }
  return keybindings;
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

class Menu : public QMenu
{
public:
  explicit Menu(const QString& title) : QMenu(title) { installEventFilter(this); }
protected:
  bool eventFilter(QObject* o, QEvent* e) override
  {
    if (o == this && e->type() == QEvent::Type::MouseMove) {
      auto* a = dynamic_cast<omm::Action*>(actionAt(static_cast<QMouseEvent*>(e)->pos()));
      if (m_current_highlighted != nullptr) { m_current_highlighted->set_highlighted(false); }
      if (a != nullptr) { a->set_highlighted(true); }
      m_current_highlighted = a;
    }
    return QMenu::eventFilter(o, e);
  }
private:
  omm::Action* m_current_highlighted = nullptr;
};

std::unique_ptr<QMenu> add_menu(const std::string& path, std::map<std::string, QMenu*>& menu_map)
{
  if (menu_map.count(path) > 0) {
    return nullptr;
  } else {
    const auto [ rest_path, menu_name ] = split(path);
    const auto menu_label = QCoreApplication::translate("menu_name", menu_name.c_str());
    std::unique_ptr<QMenu> menu = std::make_unique<Menu>(menu_label);
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

KeyBindings::KeyBindings()
  : m_keybindings(load_default_keybindings("://default_keybindings.cfg"))
{
  restore();
}

KeyBindings::~KeyBindings() { store(); }

void KeyBindings::restore()
{
  beginResetModel();
  QSettings settings;
  if (settings.childGroups().contains(keybindings_group)) {
    settings.beginGroup(keybindings_group);
    for (ContextKeyBindings& context_keybindings : m_keybindings) {
      settings.beginGroup(QString::fromStdString(context_keybindings.name));
      for (KeyBinding& keybinding : context_keybindings.key_bindings) {
        const QString action_name_qs = QString::fromStdString(keybinding.name);
        if (settings.contains(action_name_qs)) {
          const QString sequence = settings.value(action_name_qs).toString();
          keybinding.set_key_sequence(QKeySequence(sequence, QKeySequence::PortableText));
        } else {
          // keep default sequence
        }
      }
      settings.endGroup();
    }
    settings.endGroup();
  }
  endResetModel();
}

std::string KeyBindings::find_action(const std::string& context, const QKeySequence& sequence) const
{
  const ContextKeyBindings& c = *std::find_if(m_keybindings.begin(), m_keybindings.end(),
                                              [context](const ContextKeyBindings& c)
  {
    return c.name == context;
  });

  const auto it = std::find_if(c.key_bindings.begin(), c.key_bindings.end(),
                               [sequence](const KeyBinding& k) { return k.matches(sequence); });

  if (it == c.key_bindings.end()) {
    return "";
  } else {
    return it->name;
  }
}

void KeyBindings::reset()
{
  for (ContextKeyBindings& c : m_keybindings) {
    for (KeyBinding& k : c.key_bindings) {
      k.reset();
    }
  }
}

std::map<std::string, std::map<std::string, QKeySequence>> KeyBindings::key_sequences() const
{
  std::map<std::string, std::map<std::string, QKeySequence>> map;
  for (ContextKeyBindings& context_keybindings : m_keybindings) {
    for (KeyBinding& keybinding : context_keybindings.key_bindings) {
      map[context_keybindings.name][keybinding.name] = keybinding.key_sequence();
    }
  }
  return map;
}

void KeyBindings::
set_key_sequences(const std::map<std::string, std::map<std::string, QKeySequence> >& map)
{
  beginResetModel();
  for (ContextKeyBindings& c : m_keybindings) {
    if (auto cit = map.find(c.name); cit != map.end()) {
      for (KeyBinding& k : c.key_bindings) {
        if (auto it = cit->second.find(k.name); it != cit->second.end()) {
          k.set_key_sequence(it->second);
        }
      }
    }
  }
  endResetModel();
}

void KeyBindings::store() const
{
  QSettings settings;
  settings.beginGroup(keybindings_group);
  for (const ContextKeyBindings& context_keybindings : m_keybindings) {
    settings.beginGroup(QString::fromStdString(context_keybindings.name));
    for (const KeyBinding& keybinding : context_keybindings.key_bindings) {
      settings.setValue(QString::fromStdString(keybinding.name), keybinding.key_sequence());
    }
    settings.endGroup();
  }
  settings.endGroup();
}

int KeyBindings::columnCount(const QModelIndex& parent) const { Q_UNUSED(parent) return 2; }
int KeyBindings::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid()) {
    if (static_cast<const KeyBindingTreeItem*>(parent.internalPointer())->is_context()) {
      const auto context = static_cast<const ContextKeyBindings*>(parent.internalPointer())->name;
      const auto context_keybindings = *std::find_if(m_keybindings.begin(), m_keybindings.end(),
                                                     [context](const ContextKeyBindings& c)
      {
        return c.name == context;
      });
      return context_keybindings.key_bindings.size();
    } else {
      return 0;
    }
  } else {
    return m_keybindings.size();
  }
}

QVariant KeyBindings::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }

  if (static_cast<const KeyBindingTreeItem*>(index.internalPointer())->is_context()) {
    switch (role) {
    case Qt::DisplayRole:
      switch (index.column()) {
      case 0:
        return QString::fromStdString(static_cast<const ContextKeyBindings*>(index.internalPointer())->name);
      }
    }
  } else {
    switch (role) {
    case Qt::DisplayRole:
      switch (index.column()) {
      case 0:
        return QCoreApplication::translate("", static_cast<const KeyBinding*>(index.internalPointer())->name.c_str());
      case 1:
        return static_cast<const KeyBinding*>(index.internalPointer())->key_sequence().toString(QKeySequence::NativeText);
      }
      break;
    case Qt::EditRole:
      if (index.column() == 1) {
        return static_cast<const KeyBinding*>(index.internalPointer())->key_sequence();
      }
      break;
    case Qt::BackgroundRole:
    {
      const KeyBinding* binding = static_cast<const KeyBinding*>(index.internalPointer());
      if (!binding->key_sequence().isEmpty() && collides(*binding)) {
        return QColor(Qt::red).lighter();
      }
      break;
    }
    case Qt::FontRole:
      if (index.column() == 1) {
        const KeyBinding* binding = static_cast<const KeyBinding*>(index.internalPointer());
        if (binding->default_key_sequence() != binding->key_sequence()) {
          auto font = QApplication::font();
          font.setItalic(true);
          return font;
        }
      }
    case DEFAULT_KEY_SEQUENCE_ROLE:
      if (index.column() == 1) {
        return static_cast<const KeyBinding*>(index.internalPointer())->default_key_sequence();
      }
      break;
    }
  }

  return QVariant();
}

bool KeyBindings::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (role != Qt::EditRole) {
    return false;
  } else if (index.column() != 1) {
    return false;
  } else if (static_cast<KeyBindingTreeItem*>(index.internalPointer())->is_context()) {
    return false;
  }

  KeyBinding* key_binding = static_cast<KeyBinding*>(index.internalPointer());
  key_binding->set_key_sequence(value.value<QKeySequence>());
  Q_EMIT dataChanged(index, index);
  return true;
}

Qt::ItemFlags KeyBindings::flags(const QModelIndex& index) const
{
  Qt::ItemFlags flags = Qt::ItemIsEnabled;
  if (!static_cast<KeyBindingTreeItem*>(index.internalPointer())->is_context()) {
    if (index.column() == 1) {
      flags |= Qt::ItemIsEditable;
    }
  }
  return flags;
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

bool KeyBindings::collides(const KeyBinding& candidate) const
{
  if (candidate.key_sequence().count() == 0) {
    // empty sequence never collides
    return false;
  }
  const auto cit = std::find_if(m_keybindings.begin(), m_keybindings.end(),
                                [candidate](const ContextKeyBindings& c)
  {
    return c.name == candidate.context();
  });

  const auto it = std::find_if(cit->key_bindings.begin(), cit->key_bindings.end(),
                               [candidate](const KeyBinding& k)
  {
    if (k.name == candidate.name) {
      // it's the same keybinding, not a duplicate.
      return false;
    } else if (k.key_sequence().count() == 0) {
      // empty sequence never collides
      return false;
    }
    for (int i = 0; i < std::min(k.key_sequence().count(), candidate.key_sequence().count()); ++i) {
      if (k.key_sequence()[i] != candidate.key_sequence()[i]) {
        return false;
      }
    }
    // it's another keybinding but with the same context and key sequence.
    return true;
  });

  return it != cit->key_bindings.end();
}

QModelIndex KeyBindings::context_index(const std::string& context_name) const
{
  const auto it = std::find_if(m_keybindings.begin(), m_keybindings.end(),
                               [=](const ContextKeyBindings& c)
  {
    return c.name == context_name;
  });
  const int row = std::distance(m_keybindings.begin(), it);
  return index(row, 0, QModelIndex());
}

QModelIndex KeyBindings::action_index(const std::string& context_name,
                                      const std::string& action_name) const
{
  const QModelIndex parent_index = context_index(context_name);
  ContextKeyBindings& c = m_keybindings.at(parent_index.row());
  const auto it = std::find_if(c.key_bindings.begin(), c.key_bindings.end(),
                               [action_name](const KeyBinding& k)
  {
    return k.name == action_name;
  });
  const int row = std::distance(c.key_bindings.begin(), it);
  return index(row, 0, parent_index);
}

QModelIndex KeyBindings::index(int row, int column, const QModelIndex& parent) const
{
  if (!parent.isValid()) {
    return createIndex(row, column, &m_keybindings.at(row));
  } else {
    assert (static_cast<KeyBindingTreeItem*>(parent.internalPointer())->is_context());
    auto* context = static_cast<ContextKeyBindings*>(parent.internalPointer());
    return createIndex(row, column, &context->key_bindings.at(row));
  }
}

QModelIndex KeyBindings::parent(const QModelIndex& child) const
{
  assert(child.isValid());
  if (static_cast<KeyBindingTreeItem*>(child.internalPointer())->is_context()) {
    return QModelIndex();
  } else {
    auto* key_binding = static_cast<KeyBinding*>(child.internalPointer());
    const auto it = std::find_if(m_keybindings.begin(), m_keybindings.end(),
                                 [&](const ContextKeyBindings& c)
    {
      return c.name == key_binding->context();
    });

    const int row = std::distance(m_keybindings.begin(), it);
    return createIndex(row, 0, &m_keybindings.at(row));
  }
}

std::unique_ptr<QAction>
KeyBindings::make_action(CommandInterface& context, const std::string& action_name) const
{
  const auto cit = std::find_if(m_keybindings.begin(), m_keybindings.end(),
                                [context=context.type()](const ContextKeyBindings& c)
  {
    return context == c.name;
  });

#ifndef NDEBUG
  if (cit == m_keybindings.end()) {
    LERROR << "Failed to find context " << context.type();
    LFATAL("Missing context");
  }
#endif  // NDEBUG

  const auto it = std::find_if(cit->key_bindings.begin(), cit->key_bindings.end(),
                               [action_name](const KeyBinding& k)
  {
    return action_name == k.name;
  });

#ifndef NDEBUG
  if (it == cit->key_bindings.end()) {
    LERROR << "Failed to find keybinding for " << context.type() << "::" << action_name;
    LFATAL("Missing keybinding");
  }
#endif  // NDEBUG

  auto action = std::make_unique<Action>(*it);
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
