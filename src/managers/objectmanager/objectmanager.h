#pragma once

#include "managers/itemmanager.h"
#include "managers/objectmanager/objecttreeview.h"
#include "keybindings/commandinterface.h"

namespace omm
{

class ObjectManager
  : public ItemManager<ObjectTreeView>, public CommandInterface
{

public:
  explicit ObjectManager(Scene& scene);
  static std::vector<CommandInterface::ActionInfo<ObjectManager>> action_infos();

protected:
  std::vector<std::string> application_actions() const override;
  bool child_key_press_event(QWidget& child, QKeyEvent& event) override;
  void populate_menu(QMenu& menu) override;

public:
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "ObjectManager");
  std::string type() const override;

private:
  void group_selected_objects();

};

}  // namespace omm
