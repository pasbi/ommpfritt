#pragma once

#include <QAbstractItemModel>
#include <map>
#include <memory>
#include <string>

namespace omm
{

class PreferencesTreeGroupItem;
class PreferencesTreeValueItem;

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
  PreferencesTree(const QString& translation_context, const QString& filename);
  ~PreferencesTree() override;
  PreferencesTree(PreferencesTree&&) = delete;
  PreferencesTree(const PreferencesTree&) = delete;
  PreferencesTree& operator=(PreferencesTree&&) = delete;
  PreferencesTree& operator=(const PreferencesTree&) = delete;
  void reset();

  void save_in_qsettings(const QString& q_settings_group) const;
  void load_from_qsettings(const QString& q_settings_group);
  bool save_to_file(const QString& filename) const;
  bool load_from_file(const QString& filename);

  PreferencesTreeGroupItem* group(const QString& name) const;
  std::vector<PreferencesTreeGroupItem*> groups() const;
  PreferencesTreeValueItem* value(const QString& group_name, const QString& key) const;
  QString stored_value(const QString& group_name, const QString& key, std::size_t column) const;

  void store();
  void restore();

  // == reimplemented from QAbstractItemModel
  QModelIndex group_index(const QString& group_name) const;
  QModelIndex value_index(const QString& group_name, const QString& key) const;
  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& child) const override;
  int rowCount(const QModelIndex& parent) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  static bool is_group(const QModelIndex& index);
  static PreferencesTreeGroupItem& group(const QModelIndex& index);
  static PreferencesTreeValueItem& value(const QModelIndex& index);
  static constexpr auto DEFAULT_VALUE_ROLE = Qt::UserRole + 1;
  virtual void apply();
  QString dump() const;

Q_SIGNALS:
  void data_changed(const omm::PreferencesTreeGroupItem& group);

protected:
  virtual bool set_data(int column, PreferencesTreeValueItem& item, const QVariant& value) = 0;
  virtual QVariant data(int column, const PreferencesTreeValueItem& item, int role) const = 0;

private:
  /**
   * @brief m_values it was be more intuitive to store the values in a map of maps,
   * however, this would complicate the implementation of the index access methods.
   */
  mutable std::vector<std::unique_ptr<PreferencesTreeGroupItem>> m_groups;
  std::map<QString, std::map<QString, QString>> m_stored_values;
  const QString m_translation_context;
  bool handle_group_line(const QString& group_name, const QString& line, bool insert_mode);
};

}  // namespace omm
