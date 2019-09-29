#pragma once

#include <QTreeView>
#include <set>

namespace omm
{

class Animator;
class Track;
class TrackViewDelegate;

class DopeSheetView : public QTreeView
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
};

}  // namespace
