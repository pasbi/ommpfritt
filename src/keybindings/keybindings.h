#pragma once

#include "keybindings/keybinding.h"
#include <vector>
#include <QAbstractTableModel>
#include <QTimer>
#include <QMenu>
#include <memory>
#include "keybindings/action.h"
#include <QKeyEvent>
#include <set>
#include "commandinterface.h"
#include "common.h"
#include "logging.h"
#include "managers/manager.h"
#include "cachedgetter.h"

class QKeyEvent;

namespace omm
{

class CommandInterface;

class KeyBindings : public QAbstractItemModel
{
public:
  KeyBindings();
  ~KeyBindings();

  void store() const;
  void restore();
  static constexpr auto keybindings_group = "keybindings";

  static constexpr auto SEPARATOR = "separator";

  std::unique_ptr<QAction>
  make_action(CommandInterface& context, const std::string& action_name) const;

  std::vector<std::unique_ptr<QMenu>>
  make_menus(CommandInterface& context, const std::vector<std::string>& actions) const;

  std::string find_action(const std::string& context, const QKeySequence& sequence) const;
  void reset();

  std::map<std::string, std::map<std::string, QKeySequence>> key_sequences() const;
  void set_key_sequences(const std::map<std::string, std::map<std::string, QKeySequence>>& map);

public:
  static constexpr auto DEFAULT_KEY_SEQUENCE_ROLE = Qt::UserRole + 1;

  // == reimplemented from QAbstractItemModel
  QModelIndex context_index(const std::string& context_name) const;
  QModelIndex action_index(const std::string& context_name, const std::string& action_name) const;
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &child) const override;
  int columnCount(const QModelIndex& parent) const override;
  int rowCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role) override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
  QKeySequence make_key_sequence(const QKeyEvent& event) const;

  static std::pair<std::string, QMenu*>
  get_menu( const std::string& action_path, std::map<std::string, QMenu*>& menu_map,
            std::list<std::unique_ptr<QMenu>>& menus);
  bool call_global_command(const QKeySequence& sequence, const CommandInterface& source) const;

  bool collides(const KeyBinding& candidate) const;

  mutable std::vector<std::unique_ptr<ContextKeyBindings>> m_keybindings;
};

}  // namespace omm
