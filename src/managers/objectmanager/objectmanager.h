#pragma once

#include "managers/itemmanager.h"
#include "managers/objectmanager/objecttreeview.h"

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
};

}  // namespace omm
