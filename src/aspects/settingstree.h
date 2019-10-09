#pragma once

#include <string>
#include <map>
#include <QAbstractItemModel>
#include "aspects/settingstreeitem.h"

namespace omm
{

/**
 * @brief The SettingsTree class represents an abstract store for settings.
 * A settings is identified by its group and key.
 * This mapping yields a tree with depth 1 with the group as nodes and the values as leafs.
 *
 * The groups and keys are only defined by what configuration is loaded in the constructor and
 * cannot be changed later.
 * The values of the settings, however, can be changed anytime.
 */
class SettingsTree : public QAbstractItemModel
{
  Q_OBJECT
public:
  SettingsTree(const std::string filename);
  ~SettingsTree();

  void save_in_qsettings(const std::string& q_settings_group) const;
  void load_from_qsettings(const std::string& q_settings_group);
  bool save_to_file(const std::string& filename) const;
  bool load_from_file(const std::string& filename);

  SettingsTreeGroupItem* group(const std::string& name) const;
  std::vector<SettingsTreeGroupItem*> groups() const;
  SettingsTreeValueItem* value(const std::string group_name, const std::string& key) const;
  void reset();

  void store();
  void restore();

  // == reimplemented from QAbstractItemModel
  QModelIndex group_index(const std::string& group_name) const;
  QModelIndex value_index(const std::string& group_name, const std::string& key) const;
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &child) const override;
  int rowCount(const QModelIndex& parent) const override;

private:
  /**
   * @brief m_values it was be more intuitive to store the values in a map of maps,
   * however, this would complicate the implementation of the index access methods.
   */
  mutable std::vector<std::unique_ptr<SettingsTreeGroupItem>> m_groups;
  std::map<std::string, std::map<std::string, std::string>> m_stored_values;
};

}  // namespace omm
