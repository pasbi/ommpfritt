#pragma once

#include "managers/itemmanager.h"
#include "managers/objectmanager/objecttreeview.h"
#include "keybindings/commandinterface.h"

namespace omm
{

class ObjectManager : public ItemManager<ObjectTreeView>, public CommandInterface
{

public:
  static std::map<std::string, QKeySequence> default_bindings();
  explicit ObjectManager(Scene& scene);
  void call(const std::string& command) override;

protected:
  void keyPressEvent(QKeyEvent* event) override;
  std::vector<std::string> application_actions() const override;

public:
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "ObjectManager");
  std::string type() const override;

};

}  // namespace omm
