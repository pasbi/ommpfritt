#pragma once

#include "keybindings/keybinding.h"
#include <vector>
#include <QAbstractTableModel>
#include <QTimer>

class QKeyEvent;

namespace omm
{

class CommandInterface;

class KeyBindings : public QAbstractTableModel
{
public:
  KeyBindings();
  ~KeyBindings();

  void store() const;
  void restore();
  static constexpr auto settings_group = "keybindings";

  bool call(const QKeySequence& key_sequence, CommandInterface& interface) const;
  bool call(const QKeyEvent& key_event, CommandInterface& interface) const;
  void set_global_command_interface(CommandInterface& global_command_interface);

public:
  int columnCount(const QModelIndex& parent) const override;
  int rowCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role) override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  static constexpr auto SEQUENCE_COLUMN = 2;
  static constexpr auto NAME_COLUMN = 0;
  static constexpr auto CONTEXT_COLUMN = 1;

private:
  std::vector<KeyBinding> m_bindings;
  CommandInterface* m_global_command_interface = nullptr;
  QKeySequence make_key_sequence(const QKeyEvent& event) const;
  mutable std::list<int> m_current_sequene;
  mutable QTimer m_reset_timer;
  static constexpr auto m_reset_delay = std::chrono::milliseconds(1000);

};

}  // namespace omm
