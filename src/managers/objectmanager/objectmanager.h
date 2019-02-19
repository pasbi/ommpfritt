#pragma once

#include "managers/itemmanager.h"
#include "managers/objectmanager/objecttreeview.h"
#include "keybindings/commandinterface.h"

namespace omm
{

class ObjectManager : public ItemManager<ObjectTreeView>, public CommandInterface
{
  DECLARE_MANAGER_TYPE(ObjectManager)

public:
  static std::map<std::string, QKeySequence> default_bindings();
  explicit ObjectManager(Scene& scene);
  void call(const std::string& command) override;

protected:
  void keyPressEvent(QKeyEvent* event) override;
  std::vector<std::string> application_actions() const override;

};

}  // namespace omm
