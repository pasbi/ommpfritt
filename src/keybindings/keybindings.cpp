#include "keybindings/keybindings.h"
#include <QSettings>
#include "keybindings/commandinterface.h"
#include <glog/logging.h>

namespace
{

QString settings_key(const omm::KeyBinding& binding)
{
  return QString::fromStdString(binding.context()) + "/" + QString::fromStdString(binding.name());
}

std::vector<omm::KeyBinding> default_keybindings {
  omm::KeyBinding("undo", "MainWindow", "Ctrl+Z"),
  omm::KeyBinding("redo", "MainWindow", "Ctrl+Y"),
};

}  // namespace

namespace omm
{

KeyBindings::KeyBindings(CommandInterface& global_command_interface)
  : m_bindings(default_keybindings)
  , m_global_command_interface(global_command_interface)
{
  restore();
}

KeyBindings::~KeyBindings() { store(); }

void KeyBindings::restore()
{
  QSettings settings;
  settings.beginGroup(settings_group);
  for (auto& key_binding : m_bindings) {
    if (const auto key = settings_key(key_binding); settings.contains(key)) {
      const auto sequence = settings.value(key).toString();
      key_binding.set_key_sequence(QKeySequence(sequence, QKeySequence::PortableText));
    }
  }
  settings.endGroup();
}

void KeyBindings::store() const
{
  QSettings settings;
  settings.beginGroup(settings_group);
  for (const auto& key_binding : m_bindings) {
    const auto sequence = key_binding.key_sequence().toString(QKeySequence::PortableText);
    settings.setValue(settings_key(key_binding), sequence);
  }
  settings.endGroup();
}

bool KeyBindings::call(const QKeySequence& sequence, CommandInterface& interface) const
{
  const auto context = interface.type();

  const auto is_match = [sequence, context](const auto& binding) {
    return binding.matches(sequence, context);
  };

  const auto it = std::find_if(m_bindings.begin(), m_bindings.end(), is_match);
  if (it != m_bindings.end()) {
    interface.call(it->name());
    return true;
  } else if (&interface != &m_global_command_interface) {
    call(sequence, m_global_command_interface);
    return true;
  } else {
    return false;
  }
}

int KeyBindings::columnCount(const QModelIndex& parent) const { return 3; }
int KeyBindings::rowCount(const QModelIndex& parent) const { return m_bindings.size(); }
QVariant KeyBindings::data(const QModelIndex& index, int role) const
{
  assert(index.isValid());
  const auto& binding = m_bindings[index.row()];
  switch (role) {
  case Qt::DisplayRole:
    switch (index.column()) {
    case NAME_COLUMN:
      return QString::fromStdString(binding.name());
    case CONTEXT_COLUMN:
      return QString::fromStdString(binding.context());
    case SEQUENCE_COLUMN:
      return binding.key_sequence().toString(QKeySequence::PortableText);
    }
    break;
  case Qt::EditRole:
    switch (index.column()) {
    case SEQUENCE_COLUMN:
      return binding.key_sequence();
    }
    break;
  }
  return QVariant();
}

bool KeyBindings::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (role != Qt::EditRole) { return false; }
  assert(index.column() == SEQUENCE_COLUMN);
  if (value.canConvert<QKeySequence>()) {
    m_bindings[index.row()].set_key_sequence(value.value<QKeySequence>());
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
      case NAME_COLUMN: return "name";
      case CONTEXT_COLUMN: return "context";
      case SEQUENCE_COLUMN: return "sequence";
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

}  // namespace omm
