#pragma once

#include "managers/itemmanager.h"
#include "managers/stylemanager/stylelistview.h"
#include "keybindings/commandinterface.h"

namespace omm
{

class StyleManager : public ItemManager<StyleListView>
{
  Q_OBJECT
public:
  explicit StyleManager(Scene& scene);

  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "StyleManager");
  std::string type() const override;
  bool perform_action(const std::string& action_name) override;
};

}  // namespace omm
