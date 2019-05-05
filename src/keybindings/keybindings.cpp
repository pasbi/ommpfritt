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

namespace
{

QString settings_key(const omm::KeyBinding& binding)
{
  return QString::fromStdString(binding.context()) + "/" + QString::fromStdString(binding.name());
}

template<typename CommandInterfaceT>
void collect_default_bindings(std::list<omm::KeyBinding>& bindings)
{
  for (const auto& action_info : CommandInterfaceT::action_infos()) {
    bindings.push_back(action_info.make_keybinding());
  }
}

std::vector<omm::KeyBinding> collect_default_bindings()
{
  std::list<omm::KeyBinding> default_bindings;
  collect_default_bindings<omm::Application>(default_bindings);
  collect_default_bindings<omm::StyleManager>(default_bindings);
  collect_default_bindings<omm::ObjectManager>(default_bindings);
  collect_default_bindings<omm::PythonConsole>(default_bindings);
  return std::vector(default_bindings.begin(), default_bindings.end());
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
  : m_bindings(collect_default_bindings())
{
  restore();
  m_reset_timer.setSingleShot(true);
  connect(&m_reset_timer, &QTimer::timeout, [this]() {
    m_current_sequene.clear();
  });
}

KeyBindings::~KeyBindings() { store(); }

void KeyBindings::restore()
{
  QSettings settings;
  if (settings.childGroups().contains(keybindings_group)) {
    settings.beginGroup(keybindings_group);
    for (auto& key_binding : m_bindings) {
      if (const auto key = settings_key(key_binding); settings.contains(key)) {
        const auto sequence = settings.value(key).toString();
        key_binding.set_key_sequence(QKeySequence(sequence, QKeySequence::PortableText));
      } else {
        key_binding.set_key_sequence(QKeySequence());
      }
    }
    settings.endGroup();
  }
}

void KeyBindings::store() const
{
  QSettings settings;
  settings.beginGroup(keybindings_group);
  for (const auto& key_binding : m_bindings) {
    const auto sequence = key_binding.key_sequence().toString(QKeySequence::PortableText);
    settings.setValue(settings_key(key_binding), sequence);
  }
  settings.endGroup();
}

QKeySequence KeyBindings::make_key_sequence(const QKeyEvent& event) const
{
  static constexpr std::size_t MAX_SEQUENCE_LENGTH = 4;  // must be 4 to match QKeySequence impl.
  const int code = event.key() | event.modifiers();
  m_current_sequene.push_back(code);
  if (m_current_sequene.size() > MAX_SEQUENCE_LENGTH) { m_current_sequene.pop_front(); }

  std::vector sequence(m_current_sequene.begin(), m_current_sequene.end());
  sequence.reserve(MAX_SEQUENCE_LENGTH);
  while (sequence.size() < MAX_SEQUENCE_LENGTH) { sequence.push_back(0); }

  return QKeySequence(sequence[0], sequence[1], sequence[2], sequence[3]);
}

int KeyBindings::columnCount(const QModelIndex& parent) const { Q_UNUSED(parent) return 3; }
int KeyBindings::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  return static_cast<int>(m_bindings.size());
}

QVariant KeyBindings::data(const QModelIndex& index, int role) const
{
  assert(index.isValid());
  const auto& binding = m_bindings[static_cast<std::size_t>(index.row())];
  switch (role) {
  case Qt::DisplayRole:
    switch (index.column()) {
    case NAME_COLUMN:
      return QCoreApplication::translate("any-context", binding.name().c_str());
    case CONTEXT_COLUMN:
      return QCoreApplication::translate("any-context", binding.context().c_str());
    case SEQUENCE_COLUMN:
      return binding.key_sequence().toString(QKeySequence::NativeText);
    }
    break;
  case Qt::EditRole:
    switch (index.column()) {
    case SEQUENCE_COLUMN:
      return binding.key_sequence();
    }
    break;
  case DEFAULT_KEY_SEQUENCE_ROLE:
    return binding.default_key_sequence();
  }
  return QVariant();
}

bool KeyBindings::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (role != Qt::EditRole) { return false; }
  assert(index.column() == SEQUENCE_COLUMN);
  if (value.canConvert<QKeySequence>()) {
    m_bindings[static_cast<std::size_t>(index.row())].set_key_sequence(value.value<QKeySequence>());
    return true;
  }  else {
    return false;
  }
}

QVariant KeyBindings::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal) {
    switch (role) {
    case Qt::DisplayRole:
      switch (section) {
      case NAME_COLUMN: return QObject::tr("name", "keybindings");
      case CONTEXT_COLUMN: return QObject::tr("context", "keybindings");
      case SEQUENCE_COLUMN: return QObject::tr("sequence", "keybindings");
      }
    }
    return QVariant();
  } else {
    return QVariant();
  }
}


Qt::ItemFlags KeyBindings::flags(const QModelIndex& index) const
{
  Qt::ItemFlags flags = Qt::ItemIsEnabled;
  if (index.column() == SEQUENCE_COLUMN) { flags |= Qt::ItemIsEditable; }
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

bool KeyBindings::call_global_command( const QKeySequence& sequence,
                                       const CommandInterface& source ) const
{
  auto& global_command_interface = Application::instance();
  if (&global_command_interface == &source) {
    return false;
  } else {
    return call(sequence, global_command_interface);
  }
}

}  // namespace omm
