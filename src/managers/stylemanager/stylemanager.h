#pragma once

#include "managers/itemmanager.h"
#include "managers/stylemanager/stylelistview.h"
#include "keybindings/commandinterface.h"

namespace omm
{

class StyleManager : public ItemManager<StyleListView>, public CommandInterface
{
  DECLARE_MANAGER_TYPE(StyleManager)
  Q_OBJECT
public:
  explicit StyleManager(Scene& scene);
  void call(const std::string& command) override;
  static std::map<std::string, QKeySequence> default_bindings();

protected:
  void keyPressEvent(QKeyEvent* event) override;
  std::vector<std::string> application_actions() const override;
  void populate_menu(QMenu& menu) override;
};

}  // namespace omm
