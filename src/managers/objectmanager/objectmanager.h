#pragma once

#include "managers/itemmanager.h"
#include "managers/objectmanager/objecttreeview.h"

namespace omm
{
class ObjectManager : public ItemManager<ObjectTreeView>
{
  Q_OBJECT
public:
  explicit ObjectManager(Scene& scene);
  bool perform_action(const QString& name) override;

protected:
  void contextMenuEvent(QContextMenuEvent* event) override;

public:
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "ObjectManager");
  [[nodiscard]] QString type() const override;
};

}  // namespace omm
