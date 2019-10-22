#pragma once

#include <QTreeView>
#include <set>
#include "managers/timeline/timelinecanvas.h"
#include "widgets/itemproxyview.h"

namespace omm
{

class Animator;
class Track;
class TrackViewDelegate;

class DopeSheetView : public ItemProxyView<QTreeView>
{
  Q_OBJECT;
public:
  explicit DopeSheetView(Animator& animator);

public Q_SLOTS:
  void update_second_column(Track& track);
  void update_second_column();

protected:
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
  Animator& m_animator;
  TrackViewDelegate* m_track_view_delegate;

  class TimelineCanvasC : public TimelineCanvas
  {
  public:
    TimelineCanvasC(Animator& animator, DopeSheetView& self);
    void update() override;
    QPoint map_to_global(const QPoint &pos) const override;
    void enable_context_menu() override;
    void disable_context_menu() override;
    QRect track_rect(Track &track) override;
    QRect owner_rect(AbstractPropertyOwner &owner) override;
  private:
    DopeSheetView& m_self;
  };

  mutable TimelineCanvasC m_canvas;
};

}  // namespace
