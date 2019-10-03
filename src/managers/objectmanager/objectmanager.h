#pragma once

#include "managers/itemmanager.h"
#include "managers/objectmanager/objecttreeview.h"
#include "keybindings/commandinterface.h"

namespace omm
{

class ObjectManager : public ItemManager<ObjectTreeView>
{
  Q_OBJECT
public:
  explicit ObjectManager(Scene& scene);
  bool perform_action(const std::string& name) override;

public:
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "ObjectManager");
  std::string type() const override;

};

}  // namespace omm
