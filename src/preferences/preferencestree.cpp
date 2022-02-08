#include "preferences/preferencestree.h"
#include "logging.h"
#include "preferences/preferencestreeitem.h"
#include <QApplication>
#include <QCoreApplication>
#include <QFile>
#include <QPalette>
#include <QRegularExpression>
#include <QSettings>

namespace omm
{
PreferencesTree::PreferencesTree(const QString& translation_context, const QString& filename)
    : m_translation_context(translation_context)
{
  if (!load_from_file(filename)) {
    LERROR << "Failed to read file '" << filename << "'.";
    LFATAL("Failed to read default settings file.");
  } else {
    LINFO << "initialized SettingTree from '" << filename << "': " << m_groups.size() << " groups.";
  }

  connect(this, &PreferencesTree::data_changed, [this](const PreferencesTreeGroupItem& group) {
    const int n = columnCount(group_index(group.name)) - 1;
    const QModelIndex tl = value_index(group.name, group.values.front()->name).siblingAtColumn(1);
    const QModelIndex br = value_index(group.name, group.values.back()->name).siblingAtColumn(n);
    Q_EMIT dataChanged(tl, br);
  });
}

PreferencesTree::~PreferencesTree() = default;

void PreferencesTree::reset()
{
  for (auto&& group : groups()) {
    for (auto&& value : group->values) {
      value->reset();
    }
    Q_EMIT data_changed(*group);
  }
}

void PreferencesTree::save_in_qsettings(const QString& q_settings_group) const
{
  QSettings settings;
  settings.beginGroup(q_settings_group);
  for (const std::unique_ptr<PreferencesTreeGroupItem>& group : m_groups) {
    settings.beginGroup(group->name);
    for (const std::unique_ptr<PreferencesTreeValueItem>& value : group->values) {
      settings.setValue(value->name, value->value());
    }
    settings.endGroup();
  }
  settings.endGroup();
}

void PreferencesTree::load_from_qsettings(const QString& q_settings_group)
{
  const auto& settings_group = q_settings_group;
  QSettings settings;
  if (settings.childGroups().contains(settings_group)) {
    settings.beginGroup(settings_group);
    for (std::unique_ptr<PreferencesTreeGroupItem>& group : m_groups) {
      settings.beginGroup(group->name);
      for (std::unique_ptr<PreferencesTreeValueItem>& value_item : group->values) {
        const QString key = value_item->name;
        if (settings.contains(key)) {
          const QString value = settings.value(key).toString();
          value_item->set_value(value);
        } else {
          // keep default value
        }
      }
      Q_EMIT data_changed(*group);
      settings.endGroup();
    }
    settings.endGroup();
  }
}

bool PreferencesTree::save_to_file(const QString& filename) const
{
  QFile file(filename);
  if (!file.open(QIODevice::WriteOnly)) {
    LERROR << "failed to open file '" << filename << "'.";
    return false;
  }

  file.write(dump().toUtf8());
  return true;
}

bool PreferencesTree::handle_group_line(const QString& group_name,
                                        const QString& line,
                                        bool insert_mode)
{
  const auto tokens = line.split(":");
  if (tokens.size() != 2) {
    LWARNING << "ignoring line '" << line << "'. Expected format: <name>: <key value>.";
    return false;
  }
  const auto name = tokens[0].trimmed();
  const auto value = tokens[1].trimmed();
  const auto git
      = std::find_if(m_groups.begin(),
                     m_groups.end(),
                     [group_name](const std::unique_ptr<PreferencesTreeGroupItem>& group) {
                       return group->name == group_name;
                     });

  PreferencesTreeGroupItem* group = nullptr;
  if (git == m_groups.end()) {
    if (insert_mode) {
      auto item = std::make_unique<PreferencesTreeGroupItem>(group_name, m_translation_context);
      m_groups.push_back(std::move(item));
    } else {
      LWARNING << "Ignore unexpected group '" << group_name << "'.";
      return false;
    }
    group = m_groups.back().get();
  } else {
    group = git->get();
  }

  const auto vit = std::find_if(group->values.begin(),
                                group->values.end(),
                                [name](auto&& value_item) { return value_item->name == name; });

  if (vit == group->values.end()) {
    if (insert_mode) {
      auto item = std::make_unique<PreferencesTreeValueItem>(group->name,
                                                             name,
                                                             value,
                                                             m_translation_context);
      group->values.push_back(std::move(item));
    } else {
      LWARNING << "No such item '" << group_name << "'::'" << name << "'.";
    }
  } else {
    if (insert_mode) {
      LWARNING << "Duplicate value for '" << group_name << "'::'" << name << "'."
               << "Drop '" << value << "', "
               << "keep '" << (*vit)->value() << "'.";
      LFATAL("Duplicate key.");
    } else {
      (*vit)->set_default(value);
    }
  }

  return true;
}

bool PreferencesTree::load_from_file(const QString& filename)
{
  const bool insert_mode = m_groups.empty();
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly)) {
    LERROR << "Failed to open file '" << filename << "'.";
    return false;
  }

  struct ResetModel {
    ResetModel(const std::function<void()>& begin_reset, const std::function<void()>& end_reset)
        : m_end_reset(end_reset)
    {
      begin_reset();
    }

    ~ResetModel()
    {
      m_end_reset();
    }

    ResetModel(ResetModel&&) = delete;
    ResetModel(const ResetModel&) = delete;
    ResetModel& operator=(ResetModel&&) = delete;
    ResetModel& operator=(const ResetModel&) = delete;

  private:
    const std::function<void()> m_end_reset;
  };

  // If we're in insert mode, create a RAII-guard to reset the model.
  const std::unique_ptr<ResetModel> reseter
      = insert_mode ? std::make_unique<ResetModel>([this]() { beginResetModel(); },
                                                   [this]() { endResetModel(); })
                    : std::unique_ptr<ResetModel>(nullptr);

  static const QRegularExpression context_regexp(QRegularExpression::anchoredPattern(R"(\[\w+\])"));
  QString group_name = "";

  QTextStream stream(&file);
  while (!stream.atEnd()) {
    const QString line = stream.readLine().trimmed();
    if (line.startsWith("#") || line.isEmpty()) {
      continue;  // line is a comment
    }

    if (context_regexp.match(line).hasMatch()) {
      group_name = line.mid(1, line.size() - 2);
    } else if (!group_name.isEmpty()) {
      if (!handle_group_line(group_name, line, insert_mode)) {
        continue;
      }
    } else {
      LWARNING << "line '" << line << "' ignored since no group is active.";
    }
  }

  if (!insert_mode) {
    for (auto&& group : groups()) {
      Q_EMIT data_changed(*group);
    }
  }
  return true;
}

PreferencesTreeGroupItem* PreferencesTree::group(const QString& name) const
{
  const auto git = std::find_if(m_groups.begin(), m_groups.end(), [name](const auto& group) {
    return group->name == name;
  });
  if (git == m_groups.end()) {
    return nullptr;
  } else {
    return git->get();
  }
}

std::vector<PreferencesTreeGroupItem*> PreferencesTree::groups() const
{
  return util::transform(m_groups, [](const auto& g) { return g.get(); });
}

PreferencesTreeValueItem* PreferencesTree::value(const QString& group_name,
                                                 const QString& key) const
{
  const auto* group = this->group(group_name);
  const auto vit = std::find_if(group->values.begin(),
                                group->values.end(),
                                [key](const auto& value) { return value->name == key; });
  return vit->get();
}

QString PreferencesTree::stored_value(const QString& group_name,
                                      const QString& key,
                                      std::size_t column) const
{
  return PreferencesTreeValueItem::value(m_stored_values.at(group_name).at(key), column);
}

void PreferencesTree::store()
{
  for (const std::unique_ptr<PreferencesTreeGroupItem>& group : m_groups) {
    for (const std::unique_ptr<PreferencesTreeValueItem>& value : group->values) {
      m_stored_values[group->name][value->name] = value->value();
    }
  }
}

void PreferencesTree::restore()
{
  for (const std::unique_ptr<PreferencesTreeGroupItem>& group : m_groups) {
    for (const std::unique_ptr<PreferencesTreeValueItem>& value : group->values) {
      value->set_value(m_stored_values[group->name][value->name]);
    }
  }
}

QModelIndex PreferencesTree::group_index(const QString& group_name) const
{
  const auto git = std::find_if(m_groups.begin(), m_groups.end(), [group_name](const auto& group) {
    return group->name == group_name;
  });
  const int row = std::distance(m_groups.begin(), git);
  return index(row, 0, QModelIndex());
}

QModelIndex PreferencesTree::value_index(const QString& group_name, const QString& key) const
{
  const QModelIndex parent_index = group_index(group_name);
  PreferencesTreeGroupItem& group = *m_groups.at(parent_index.row());
  const auto vit = std::find_if(group.values.begin(), group.values.end(), [key](const auto& value) {
    return value->name == key;
  });
  const int row = std::distance(group.values.begin(), vit);
  return index(row, 0, parent_index);
}

QModelIndex PreferencesTree::index(int row, int column, const QModelIndex& parent) const
{
  PreferencesTreeItem* internal_pointer = nullptr;
  if (!parent.isValid()) {
    internal_pointer = m_groups.at(row).get();
  } else {
    const auto* ptr = static_cast<const PreferencesTreeItem*>(parent.internalPointer());
    assert(ptr->is_group());
    const auto* group = dynamic_cast<const PreferencesTreeGroupItem*>(ptr);
    internal_pointer = group->values.at(row).get();
  }
  return createIndex(row, column, internal_pointer);
}

QModelIndex PreferencesTree::parent(const QModelIndex& child) const
{
  assert(child.isValid());
  if (static_cast<PreferencesTreeItem*>(child.internalPointer())->is_group()) {
    return QModelIndex();
  } else {
    auto* const ptr = static_cast<PreferencesTreeItem*>(child.internalPointer());
    auto* value_item = dynamic_cast<PreferencesTreeValueItem*>(ptr);
    const auto it = std::find_if(m_groups.begin(), m_groups.end(), [&](const auto& group) {
      return group->name == value_item->group;
    });

    const int row = std::distance(m_groups.begin(), it);
    return createIndex(row, 0, m_groups.at(row).get());
  }
}

int PreferencesTree::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid()) {
    auto* ptr = static_cast<PreferencesTreeItem*>(parent.internalPointer());
    if (ptr->is_group()) {
      return group(ptr->name)->values.size();
    } else {
      return 0;
    }
  } else {
    return m_groups.size();
  }
}

int PreferencesTree::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  return 2;
}

QVariant PreferencesTree::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }

  if (role == Qt::ForegroundRole) {
    return QApplication::palette().color(QPalette::Active, QPalette::WindowText);
  }

  if (is_group(index)) {
    switch (index.column()) {
    case 0:
      switch (role) {
      case Qt::DisplayRole:
        return group(index).translated_name();
      default:
        return QVariant();
      }
    default:
      return QVariant();
    }
  } else {
    switch (index.column()) {
    case 0:
      switch (role) {
      case Qt::DisplayRole:
        return value(index).translated_name();
      case Qt::DecorationRole:
        return value(index).icon();
      default:
        return QVariant();
      }
    default:
      return data(index.column(), value(index), role);
    }
  }
}

bool PreferencesTree::setData(const QModelIndex& index, const QVariant& value, int role)
{
  auto* ptr = static_cast<PreferencesTreeItem*>(index.internalPointer());
  if (role != Qt::EditRole || index.column() == 0 || ptr->is_group()) {
    return false;
  }

  auto* value_item = dynamic_cast<PreferencesTreeValueItem*>(ptr);
  if (set_data(index.column(), *value_item, value)) {
    Q_EMIT dataChanged(index, index);
    return true;
  } else {
    return false;
  }
}

Qt::ItemFlags PreferencesTree::flags(const QModelIndex& index) const
{
  Qt::ItemFlags flags = Qt::ItemIsEnabled;
  if (!static_cast<PreferencesTreeItem*>(index.internalPointer())->is_group()) {
    if (index.column() >= 1) {
      flags |= Qt::ItemIsEditable;
    }
  }
  return flags;
}

bool PreferencesTree::is_group(const QModelIndex& index)
{
  return static_cast<const PreferencesTreeItem*>(index.internalPointer())->is_group();
}

PreferencesTreeGroupItem& PreferencesTree::group(const QModelIndex& index)
{
  assert(is_group(index));
  auto* ptr = static_cast<PreferencesTreeItem*>(index.internalPointer());
  return *dynamic_cast<PreferencesTreeGroupItem*>(ptr);
}

PreferencesTreeValueItem& PreferencesTree::value(const QModelIndex& index)
{
  assert(!is_group(index));
  auto* ptr = static_cast<PreferencesTreeItem*>(index.internalPointer());
  return *dynamic_cast<PreferencesTreeValueItem*>(ptr);
}

void PreferencesTree::apply()
{
  store();
}

QString PreferencesTree::dump() const
{
  QString dump;
  for (auto&& group : m_groups) {
    dump += QString("[%1]\n").arg(group->name);
    for (auto&& value : group->values) {
      dump += value->name + ":" + value->value() + "\n";
    }
  }
  return dump;
}

}  // namespace omm
