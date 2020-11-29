#pragma once

#include "common.h"
#include "preferences/preferencestree.h"
#include <memory>
#include <set>
#include <vector>

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
  ~KeyBindings() override;
  KeyBindings(KeyBindings&&) = delete;
  KeyBindings(const KeyBindings&) = delete;
  KeyBindings& operator=(KeyBindings&&) = delete;
  KeyBindings& operator=(const KeyBindings&) = delete;

  static constexpr auto TRANSLATION_CONTEXT = "keybindings";
  static constexpr auto SEPARATOR = "separator";

  std::unique_ptr<QAction> make_menu_action(CommandInterface& context,
                                            const QString& action_name) const;

  std::unique_ptr<QAction> make_toolbar_action(CommandInterface& context,
                                               const QString& action_name) const;

  std::vector<std::unique_ptr<QMenu>> make_menus(CommandInterface& context,
                                                 const std::vector<QString>& actions) const;

  QString find_action(const QString& context, const QKeySequence& sequence) const;
  const PreferencesTreeValueItem* find_action(const QString& context,
                                              const QString& action_name) const;

public:
  QVariant data(int column, const PreferencesTreeValueItem& item, int role) const override;
  bool set_data(int column, PreferencesTreeValueItem& item, const QVariant& value) override;

private:
  QKeySequence make_key_sequence(const QKeyEvent& event) const;

  static std::pair<QString, QMenu*> get_menu(const QString& action_path,
                                             std::map<QString, QMenu*>& menu_map,
                                             std::list<std::unique_ptr<QMenu>>& menus);
  bool call_global_command(const QKeySequence& sequence, const CommandInterface& source) const;

  bool collides(const PreferencesTreeValueItem& candidate) const;

  enum class Target { ToolBar, Menu };
  std::unique_ptr<QAction>
  make_action(CommandInterface& context, const QString& action_name, Target target) const;
};

}  // namespace omm
