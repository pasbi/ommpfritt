#pragma once

#include <string>
#include <map>
#include <QAbstractItemModel>
#include "preferences/preferencestreeitem.h"

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
class PreferencesTree : public QAbstractItemModel
{
  Q_OBJECT
public:
  PreferencesTree(const std::string filename);
  ~PreferencesTree();
  void reset();

  void save_in_qsettings(const std::string& q_settings_group) const;
  void load_from_qsettings(const std::string& q_settings_group);
  bool save_to_file(const std::string& filename) const;
  bool load_from_file(const std::string& filename);

  PreferencesTreeGroupItem* group(const std::string& name) const;
  std::vector<PreferencesTreeGroupItem*> groups() const;
  PreferencesTreeValueItem* value(const std::string group_name, const std::string& key) const;

  void store();
  void restore();

  // == reimplemented from QAbstractItemModel
  QModelIndex group_index(const std::string& group_name) const;
  QModelIndex value_index(const std::string& group_name, const std::string& key) const;
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &child) const override;
  int rowCount(const QModelIndex& parent) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  bool is_group(const QModelIndex& index) const;
  PreferencesTreeGroupItem& group(const QModelIndex& index) const;
  PreferencesTreeValueItem& value(const QModelIndex& index) const;
  static constexpr auto DEFAULT_VALUE_ROLE = Qt::UserRole + 1;

protected:
  virtual bool set_data(int column, PreferencesTreeValueItem& item, const QVariant& value) = 0;
  virtual QVariant data(int column, const PreferencesTreeValueItem& item, int role) const = 0;

private:
  /**
   * @brief m_values it was be more intuitive to store the values in a map of maps,
   * however, this would complicate the implementation of the index access methods.
   */
  mutable std::vector<std::unique_ptr<PreferencesTreeGroupItem>> m_groups;
  std::map<std::string, std::map<std::string, std::string>> m_stored_values;
};

}  // namespace omm
