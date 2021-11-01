#pragma once

#include "animation/animator.h"
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
  [[nodiscard]] QString type() const override
  {
    return TYPE;
  }
  bool perform_action(const QString& name) override;

public:
  void update_play_pause_button(Animator::PlayDirection direction);

private:
  Slider* m_slider;
  TimeLineTitleBar* m_title_bar;
  void jump_to_next_keyframe(Animator::PlayDirection direction);
};

}  // namespace omm
