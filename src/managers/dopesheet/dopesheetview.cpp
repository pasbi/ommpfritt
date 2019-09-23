#include "managers/dopesheet/dopesheetview.h"
#include <QPainter>
#include <QHeaderView>
#include "managers/dopesheet/trackviewdelegate.h"
#include <memory>
#include "animation/animator.h"
#include "logging.h"
#include "animation/track.h"
#include "properties/property.h"

namespace omm
{

DopeSheetView::DopeSheetView(Animator& animator) : m_animator(animator)
{
  setModel(&animator);
  header()->hide();
  auto track_view_delegate = std::make_unique<TrackViewDelegate>(m_animator);
  auto& tvdr = *track_view_delegate;
  setItemDelegateForColumn(1, track_view_delegate.release());
  connect(&m_animator, SIGNAL(start_changed(int)), this, SLOT(update_second_column()));
  connect(&m_animator, SIGNAL(end_changed(int)), this, SLOT(update_second_column()));
  connect(&m_animator, SIGNAL(current_changed(int)), this, SLOT(update_second_column()));
  connect(&m_animator, &Animator::key_inserted, [this, &tvdr](Track& track, int frame) {
    tvdr.insert_keyframe(track, frame);
    update_second_column(track);
  });
  connect(&m_animator, &Animator::key_removed, [this, &tvdr](Track& track, int frame) {
    tvdr.remove_keyframe(track, frame);
    update_second_column(track);
  });
  connect(&m_animator, &Animator::key_moved, [this, &tvdr](Track& track, int old_frame, int new_frame) {
    tvdr.move_keyframe(track, old_frame, new_frame);
    update_second_column(track);
  });
  connect(&m_animator, &Animator::track_inserted, [this, &tvdr](Track& track) {
    tvdr.insert_track(track);
    update_second_column();
  });
  connect(&m_animator, &Animator::track_removed, [this, &tvdr](Track& track) {
    tvdr.remove_track(track);
    update_second_column();
  });
  connect(&m_animator, &Animator::modelReset, [this, &tvdr]() {
    tvdr.invalidate_cache();
    update_second_column();
  });
  tvdr.invalidate_cache();
}

void DopeSheetView::update_second_column(Track& track)
{
  QModelIndex index = m_animator.index(track.property());
  viewport()->update(visualRect(index.siblingAtColumn(1)));
//  update_second_column();
}

void DopeSheetView::update_second_column()
{
  const QRect rect(QPoint(columnViewportPosition(1), 0), QSize(columnWidth(1), height()));
  viewport()->update(rect);
}

}  // namespace
