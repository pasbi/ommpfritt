#pragma once

#include "keybindings/keybinding.h"
#include <vector>
#include <QAbstractTableModel>

namespace omm
{

class CommandInterface;

class KeyBindings : public QAbstractTableModel
{
public:
  KeyBindings(CommandInterface& global_command_interface);
  ~KeyBindings();

  void store() const;
  void restore();
  static constexpr auto settings_group = "keybindings";

  bool call(const QKeySequence& key_sequence, CommandInterface& interface) const;

public:
  int columnCount(const QModelIndex& parent) const override;
  int rowCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role) override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  static constexpr auto SEQUENCE_COLUMN = 2;
  static constexpr auto NAME_COLUMN = 1;
  static constexpr auto CONTEXT_COLUMN = 0;

private:
  std::vector<KeyBinding> m_bindings;
  CommandInterface& m_global_command_interface;

};

}  // namespace omm
