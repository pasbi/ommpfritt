#include "managers/dopesheet/dopesheetview.h"
#include <QPainter>
#include <QHeaderView>
#include <QMouseEvent>
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
  auto track_view_delegate = std::make_unique<TrackViewDelegate>(*this, m_animator);
  m_track_view_delegate = track_view_delegate.get();
  setItemDelegateForColumn(1, track_view_delegate.release());
  connect(&m_animator, SIGNAL(start_changed(int)), this, SLOT(update_second_column()));
  connect(&m_animator, SIGNAL(end_changed(int)), this, SLOT(update_second_column()));
  connect(&m_animator, SIGNAL(current_changed(int)), this, SLOT(update_second_column()));
  connect(&m_animator, SIGNAL(track_changed(Track&)), this, SLOT(update_second_column()));
}

void DopeSheetView::update_second_column(Track& track)
{
  QModelIndex index = m_animator.index(track.property());
  viewport()->update(visualRect(index.siblingAtColumn(1)));
}

void DopeSheetView::update_second_column()
{
  const QRect rect(QPoint(columnViewportPosition(1), 0), QSize(columnWidth(1), height()));
  viewport()->update(rect);
}

void DopeSheetView::mouseReleaseEvent(QMouseEvent* event)
{
  m_track_view_delegate->mouse_release(*event);
}

void DopeSheetView::mousePressEvent(QMouseEvent* event)
{
  if (columnAt(event->x()) == 1) {
    m_track_view_delegate->mouse_press(*event);
  } else {
    QTreeView::mousePressEvent(event);
  }
}

void DopeSheetView::mouseMoveEvent(QMouseEvent* event)
{
  if (columnAt(event->x()) == 1) {
    m_track_view_delegate->mouse_move(*event);
  } else {
    QTreeView::mouseMoveEvent(event);
  }
}

void DopeSheetView::keyPressEvent(QKeyEvent* event)
{
  m_track_view_delegate->key_press(*event);
  QTreeView::keyPressEvent(event);
}

}  // namespace
