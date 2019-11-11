#pragma once

#include <QWidget>
#include "managers/timeline/timelinecanvas.h"

namespace omm
{

class Manager;

class CurveManagerWidget : public QWidget
{
  Q_OBJECT
public:
  explicit CurveManagerWidget(Scene& scene);
  void set_selection(const std::set<AbstractPropertyOwner*>& selection);

protected:
  void paintEvent(QPaintEvent* event);
  bool event(QEvent* event);

  struct TimelineCanvasC : public CurveTimelineCanvas
  {
    TimelineCanvasC(Animator& animator, QWidget& widget);
    void disable_context_menu() override;
    void enable_context_menu() override;
    QPoint map_to_global(const QPoint& pos) const override;
    void update() override;
    QRect track_rect(Track& track) override;
    QRect owner_rect(AbstractPropertyOwner& owner) override;
  private:
    QWidget& m_self;
  };

  TimelineCanvasC m_canvas;
};

}  // namespace omm
