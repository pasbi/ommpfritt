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
  make_action(CommandInterface& context, const std::string& action_name) const;

  std::vector<std::unique_ptr<QMenu>>
  make_menus(CommandInterface& context, const std::vector<std::string>& actions) const;

  std::string find_action(const std::string& context, const QKeySequence& sequence) const;

public:
  QVariant data(int column, const PreferencesTreeValueItem &item, int role) const override;
  bool set_data(int column, PreferencesTreeValueItem &item, const QVariant &value) override;

protected:
  std::string translate(const std::string& group, const std::string& text) const override;

private:
  QKeySequence make_key_sequence(const QKeyEvent& event) const;

  static std::pair<std::string, QMenu*>
  get_menu( const std::string& action_path, std::map<std::string, QMenu*>& menu_map,
            std::list<std::unique_ptr<QMenu>>& menus);
  bool call_global_command(const QKeySequence& sequence, const CommandInterface& source) const;

  bool collides(const PreferencesTreeValueItem& candidate) const;
};

}  // namespace omm
