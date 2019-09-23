#pragma once

#include "managers/stylemanager/stylelistview.h"
#include "keybindings/commandinterface.h"
#include "managers/manager.h"

namespace Ui { class TimeLine; }

namespace omm
{

class TimeLine : public Manager, public CommandInterface
{
  Q_OBJECT
public:
  explicit TimeLine(Scene& scene);
  static std::vector<CommandInterface::ActionInfo<TimeLine>> action_infos();

  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "TimeLine");
  std::string type() const override { return TYPE; }

private:
  struct UiTimeLineDeleter
  {
    void operator()(::Ui::TimeLine* ui);
  };
  std::unique_ptr<::Ui::TimeLine, UiTimeLineDeleter> m_ui;

};

}  // namespace omm
