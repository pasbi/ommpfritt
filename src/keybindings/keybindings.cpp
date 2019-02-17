#include "keybindings/keybindings.h"
#include <QSettings>
#include "keybindings/commandinterface.h"
#include <glog/logging.h>
#include <QKeyEvent>
#include "common.h"
#include <map>

#include "mainwindow/mainwindow.h"

namespace
{

QString settings_key(const omm::KeyBinding& binding)
{
  return QString::fromStdString(binding.context()) + "/" + QString::fromStdString(binding.name());
}

template<typename CommandInterfaceT>
void collect_default_bindings(std::list<omm::KeyBinding>& bindings)
{
  for (const auto& [ name, key_sequence ] : CommandInterfaceT::DEFAULT_BINDINGS) {
    bindings.push_back(omm::KeyBinding(name, CommandInterfaceT::TYPE, key_sequence));
  }
}

std::vector<omm::KeyBinding> collect_default_bindings()
{
  std::list<omm::KeyBinding> default_bindings;
  collect_default_bindings<omm::MainWindow>(default_bindings);
  return std::vector(default_bindings.begin(), default_bindings.end());
}

}  // namespace

namespace omm
{

KeyBindings::KeyBindings(CommandInterface& global_command_interface)
  : m_bindings(collect_default_bindings())
  , m_global_command_interface(global_command_interface)
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

bool KeyBindings::call(const QKeyEvent& key_event, CommandInterface& interface) const
{
  m_reset_timer.start(m_reset_delay);
  // QKeySequence does not support combinations without non-modifier keys.
  static const auto bad_keys = std::set { Qt::Key_unknown, Qt::Key_Shift, Qt::Key_Control,
                                          Qt::Key_Meta, Qt::Key_Alt };

  if (::contains(bad_keys, key_event.key())) {
    return false;
  } if (key_event.key() == Qt::Key_Escape) {
    m_current_sequene.clear();
    return false;
  } else if (const auto sequence = make_key_sequence(key_event); call(sequence, interface)) {
    m_current_sequene.clear();
    return true;
  } else {
    LOG(INFO) << "key sequence was not (yet) accepted: " << sequence.toString().toStdString();
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
      case NAME_COLUMN: return tr("name");
      case CONTEXT_COLUMN: return tr("context");
      case SEQUENCE_COLUMN: return tr("sequence");
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
