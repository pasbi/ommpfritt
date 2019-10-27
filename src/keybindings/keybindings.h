#pragma once

#include <vector>
#include <memory>
#include <set>
#include "common.h"
#include "preferences/preferencestree.h"

class QKeyEvent;
class QAction;
class QMenu;

namespace omm
{

class CommandInterface;

class KeyBindings : public PreferencesTree
{
public:
  KeyBindings();
  ~KeyBindings();

  static constexpr auto keybindings_group = "keybindings";
  static constexpr auto SEPARATOR = "separator";

  std::unique_ptr<QAction>
  make_action(CommandInterface& context, const QString& action_name) const;

  std::vector<std::unique_ptr<QMenu>>
  make_menus(CommandInterface& context, const std::vector<QString>& actions) const;

  QString find_action(const QString& context, const QKeySequence& sequence) const;

public:
  QVariant data(int column, const PreferencesTreeValueItem &item, int role) const override;
  bool set_data(int column, PreferencesTreeValueItem &item, const QVariant &value) override;

protected:
  QString translation_context() const override { return "keybindings"; }

private:
  QKeySequence make_key_sequence(const QKeyEvent& event) const;

  static std::pair<QString, QMenu*>
  get_menu( const QString& action_path, std::map<QString, QMenu*>& menu_map,
            std::list<std::unique_ptr<QMenu>>& menus);
  bool call_global_command(const QKeySequence& sequence, const CommandInterface& source) const;

  bool collides(const PreferencesTreeValueItem& candidate) const;
};

}  // namespace omm
