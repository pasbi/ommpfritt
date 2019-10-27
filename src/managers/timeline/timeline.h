#pragma once

#include "managers/stylemanager/stylelistview.h"
#include "keybindings/commandinterface.h"
#include "managers/manager.h"

namespace omm
{

class Slider;
class TimeLineTitleBar;

class TimeLine : public Manager
{
  Q_OBJECT
public:
  explicit TimeLine(Scene& scene);

  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "TimeLine");
  QString type() const override { return TYPE; }
  bool perform_action(const QString &name) override;

public Q_SLOTS:
  void update_play_pause_button(bool play);

private:
  Slider* m_slider;
  TimeLineTitleBar* m_title_bar;

};

}  // namespace omm
