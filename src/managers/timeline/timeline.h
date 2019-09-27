#pragma once

#include "managers/stylemanager/stylelistview.h"
#include "keybindings/commandinterface.h"
#include "managers/manager.h"

namespace omm
{

class Slider;
class TimeLineTitleBar;

class TimeLine : public Manager, public CommandInterface
{
  Q_OBJECT
public:
  explicit TimeLine(Scene& scene);
  static std::vector<CommandInterface::ActionInfo<TimeLine>> action_infos();

  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "TimeLine");
  std::string type() const override { return TYPE; }

public Q_SLOTS:
  void update_play_pause_button(bool play);

private:
  Slider* m_slider;
  TimeLineTitleBar* m_header;

};

}  // namespace omm
