#pragma once

#include "managers/timeline/timelinecanvas.h"
#include <QTreeView>
#include <set>

namespace omm
{
class Animator;
class Track;
class TrackViewDelegate;
class DopeSheetProxyModel;

class DopeSheetView : public QTreeView
{
  Q_OBJECT
public:
  explicit DopeSheetView(Animator& animator);
  ~DopeSheetView() override;

public:
  void update_second_column(omm::Track& track);
  void update_second_column();

protected:
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;

private:
  Animator& m_animator;
  TrackViewDelegate* m_track_view_delegate;
  QPoint m_mouse_press_pos;

  class TimelineCanvasC : public TimelineCanvas
  {
  public:
    TimelineCanvasC(Animator& animator, DopeSheetView& self);
    void update() override;
    [[nodiscard]] QPoint map_to_global(const QPoint& pos) const override;
    void enable_context_menu() override;
    void disable_context_menu() override;
    QRect track_rect(Track& track) override;
    QRect owner_rect(AbstractPropertyOwner& owner) override;

  private:
    DopeSheetView& m_self;
  };

  friend class TimelineCanvasC;

  mutable TimelineCanvasC m_canvas;
  std::unique_ptr<DopeSheetProxyModel> m_proxy_model;
};

}  // namespace omm
