#pragma once

#include <QWidget>
#include <set>
#include "managers/timeline/timelinecanvas.h"

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

public Q_SLOTS:
  void set_range(double left, double right);

Q_SIGNALS:
  void value_changed(int);

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void keyPressEvent(QKeyEvent *event) override;

private:
  class TimelineCanvasC : public TimelineCanvas {
  public:
    TimelineCanvasC(Animator& animator, Slider& self);
    QPoint map_to_global(const QPoint &pos) const override;
    void update() override;
  private:
    Slider& m_self;
    void update_tracks(const std::set<AbstractPropertyOwner*>& selection);
  };

  TimelineCanvasC m_canvas;
  Scene& m_scene;



};

}  // namespace
