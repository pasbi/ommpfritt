#pragma once

#include "keybindings/commandinterface.h"
#include "managers/itemmanager.h"
#include "managers/stylemanager/stylelistview.h"

namespace omm
{
class StyleManager : public ItemManager<StyleListView>
{
  Q_OBJECT
public:
  explicit StyleManager(Scene& scene);

  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "StyleManager");
  QString type() const override;
  bool perform_action(const QString& action_name) override;

protected:
  void contextMenuEvent(QContextMenuEvent* e) override;
};

}  // namespace omm
