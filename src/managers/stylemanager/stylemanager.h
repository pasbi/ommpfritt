#pragma once

#include "managers/itemmanager.h"
#include "managers/stylemanager/stylelistview.h"
#include "keybindings/commandinterface.h"

namespace omm
{

class StyleManager : public ItemManager<StyleListView>, public CommandInterface
{
  Q_OBJECT
public:
  explicit StyleManager(Scene& scene);
  static std::vector<CommandInterface::ActionInfo<StyleManager>> action_infos();

  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "StyleManager");
  std::string type() const override;

protected:
  void keyPressEvent(QKeyEvent* event) override;
  std::vector<std::string> application_actions() const override;
  void populate_menu(QMenu& menu) override;
};

}  // namespace omm
