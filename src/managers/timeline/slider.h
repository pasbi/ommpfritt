#pragma once

#include "managers/timeline/timelinecanvas.h"
#include <QWidget>
#include <set>

namespace omm
{
class Animator;
class Track;
class Scene;
class AbstractPropertyOwner;

class Slider : public QWidget
{
  Q_OBJECT
public:
  explicit Slider(Animator& animator);
  [[nodiscard]] std::set<Track*> tracks() const;

public:
  void set_range(double left, double right);

Q_SIGNALS:
  void value_changed(int);

protected:
  bool event(QEvent* event) override;
  void paintEvent(QPaintEvent* event) override;

private:
  class TimelineCanvasC : public TimelineCanvas
  {
  public:
    TimelineCanvasC(Animator& animator, Slider& self);
    [[nodiscard]] QPoint map_to_global(const QPoint& pos) const override;
    void update() override;
    void disable_context_menu() override;
    void enable_context_menu() override;
    QRect track_rect(Track& track) override;
    QRect owner_rect(AbstractPropertyOwner& owner) override;

  private:
    Slider& m_self;
  };

  TimelineCanvasC m_canvas;
  Scene& m_scene;
};

}  // namespace omm
