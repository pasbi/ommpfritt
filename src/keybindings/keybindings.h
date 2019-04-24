#pragma once

#include "keybindings/keybinding.h"
#include <vector>
#include <QAbstractTableModel>
#include <QTimer>
#include <QMenu>
#include <memory>
#include "keybindings/action.h"
#include <QCoreApplication>  // TODO remove
#include <QKeyEvent>
#include <set>
#include "commandinterface.h"
#include "common.h"
#include "logging.h"

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
  static constexpr auto keybindings_group = "keybindings";

  static constexpr auto SEPARATOR = "separator";

  template<typename CommandInterfaceT> std::unique_ptr<QAction>
  make_action(CommandInterfaceT& ci, const std::string& action_name) const
  {
    const auto it = std::find_if(m_bindings.begin(), m_bindings.end(), [&](const auto& binding) {
      return binding.name() == action_name && binding.context() == ci.type();
    });
    if (it == m_bindings.end()) {
      LERROR << "Failed to find keybindings for '" << ci.type() << "::" << action_name << "'.";
      return nullptr;
    } else {
      auto action = std::make_unique<Action>(*it);
      QObject::connect(action.get(), &QAction::triggered, [&ci, action_name] {
        ci.template call<CommandInterfaceT>(action_name);
      });
      return action;
    }
  }

  template<typename CommandInterfaceT>
  void populate_menu(QMenu& menu, CommandInterfaceT& ci)
  {
    for (auto&& action_info : CommandInterfaceT::action_infos()) {
      menu.addAction(make_action(ci, action_info.name).release());
    }
  }

  template<typename CommandInterfaceT> std::vector<std::unique_ptr<QMenu>>
  make_menus(CommandInterfaceT& ci, const std::vector<std::string>& actions) const
  {
    std::list<std::unique_ptr<QMenu>> menus;
    std::map<std::string, QMenu*> menu_map;
    for (const auto& action_path : actions) {
      auto [ action_name, menu ] = get_menu(action_path, menu_map, menus);
      if (action_name == SEPARATOR) {
        menu->addSeparator();
      } else if (!action_name.empty()) {
        menu->addAction(make_action(ci, action_name).release());
      }
    }

    return std::vector( std::make_move_iterator(menus.begin()),
                        std::make_move_iterator(menus.end()) );
  }

  template<typename CommandInterfaceT>
  bool call(const QKeySequence& sequence, CommandInterfaceT& interface) const
  {
    const auto context = interface.type();

    const auto is_match = [sequence, context](const auto& binding) {
      return binding.matches(sequence, context);
    };

    const auto it = std::find_if(m_bindings.begin(), m_bindings.end(), is_match);
    if (it != m_bindings.end()) {
      interface.template call<CommandInterfaceT>(it->name());
      return true;
    } else {
      return this->call_global_command(sequence, interface);
    }
  }

  template<typename CommandInterfaceT>
  bool call(const QKeyEvent& key_event, CommandInterfaceT& interface) const
  {
    m_reset_timer.start(m_reset_delay);
    // QKeySequence does not support combinations without non-modifier keys.
    static const auto bad_keys = std::set { Qt::Key_unknown, Qt::Key_Shift, Qt::Key_Control,
                                            Qt::Key_Meta, Qt::Key_Alt };

    if (::contains(bad_keys, key_event.key())) {
      return false;
    } else if (key_event.key() == Qt::Key_Escape) {
      m_current_sequene.clear();
      return false;
    } else {
      auto sequence = make_key_sequence(key_event);
      while (sequence.count() > 0) {
        if (call(sequence, interface)) {
          m_current_sequene.clear();
          return true;
        } else {
          // try with a suffix of the sequence. This is convenient because if user accidentaly
          // hits a key and then starts a key-sequence, that key sequence wouldn't be recognized
          // since it was prefixed with that accidental key.
          sequence = QKeySequence(sequence[1], sequence[2], sequence[3], 0);
        }
      }

      LINFO << "key sequence (and all its suffixes) were not (yet) accepted: "
            << sequence.toString().toStdString();
      return false;
    }
  }

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
  std::vector<KeyBinding> m_bindings;  // pointers must not change
  QKeySequence make_key_sequence(const QKeyEvent& event) const;
  mutable std::list<int> m_current_sequene;
  mutable QTimer m_reset_timer;
  static constexpr auto m_reset_delay = std::chrono::milliseconds(1000);

  static std::pair<std::string, QMenu*>
  get_menu( const std::string& action_path, std::map<std::string, QMenu*>& menu_map,
            std::list<std::unique_ptr<QMenu>>& menus);
  bool call_global_command(const QKeySequence& sequence, const CommandInterface& source) const;

};

}  // namespace omm
