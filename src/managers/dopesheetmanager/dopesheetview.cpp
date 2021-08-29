#include "managers/dopesheetmanager/dopesheetview.h"
#include "animation/animator.h"
#include "animation/track.h"
#include "aspects/abstractpropertyowner.h"
#include "logging.h"
#include "managers/dopesheetmanager/dopesheetheader.h"
#include "managers/dopesheetmanager/trackviewdelegate.h"
#include "managers/dopesheetmanager/dopesheetproxymodel.h"
#include "properties/property.h"
#include <QHeaderView>
#include <QMouseEvent>
#include <QPainter>
#include <memory>


namespace omm
{
DopeSheetView::DopeSheetView(Animator& animator)
  : m_animator(animator)
  , m_canvas(m_animator, *this)
  , m_proxy_model(std::make_unique<DopeSheetProxyModel>(animator))
{
  setModel(m_proxy_model.get());
  setHeader(std::make_unique<DopeSheetHeader>(m_canvas).release());
  auto track_view_delegate = std::make_unique<TrackViewDelegate>(*this, m_canvas);
  m_track_view_delegate = track_view_delegate.get();
  setItemDelegateForColumn(1, track_view_delegate.release());
  connect(&m_animator,
          &Animator::start_changed,
          this,
          qOverload<>(&DopeSheetView::update_second_column));
  connect(&m_animator,
          &Animator::end_changed,
          this,
          qOverload<>(&DopeSheetView::update_second_column));
  connect(&m_animator,
          &Animator::current_changed,
          this,
          qOverload<>(&DopeSheetView::update_second_column));
  connect(&m_animator,
          &Animator::track_changed,
          this,
          qOverload<>(&DopeSheetView::update_second_column));
  connect(&m_canvas, &TimelineCanvasC::current_frame_changed, &m_animator, &Animator::set_current);
  setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
}

DopeSheetView::~DopeSheetView() = default;

void DopeSheetView::update_second_column(Track& track)
{
  QModelIndex index = m_animator.index(track.property());
  viewport()->update(visualRect(index.siblingAtColumn(1)));
}

void DopeSheetView::update_second_column()
{
  const QRect rect(QPoint(columnViewportPosition(1), 0), QSize(columnWidth(1), height()));
  viewport()->update(rect);
  header()->viewport()->update();
}

void DopeSheetView::mouseReleaseEvent(QMouseEvent* event)
{
  const QModelIndex index = indexAt(static_cast<QMouseEvent*>(event)->pos());
  m_track_view_delegate->activate_index(index);
  m_track_view_delegate->view_event(*event);
}

void DopeSheetView::mousePressEvent(QMouseEvent* event)
{
  m_mouse_press_pos = event->pos();
  if (columnAt(event->x()) == 1) {
    const QModelIndex index = indexAt(event->pos());
    m_track_view_delegate->activate_index(index);
    if (!m_track_view_delegate->view_event(*event)) {
      QTreeView::mousePressEvent(event);
    }
  } else {
    QTreeView::mousePressEvent(event);
  }
}

void DopeSheetView::mouseMoveEvent(QMouseEvent* event)
{
  if (columnAt(m_mouse_press_pos.x()) == 1) {
    const QModelIndex index = indexAt(m_mouse_press_pos);
    m_track_view_delegate->activate_index(index);
    m_track_view_delegate->view_event(*event);
  } else {
    QTreeView::mouseMoveEvent(event);
  }
}

void DopeSheetView::keyPressEvent(QKeyEvent* event)
{
  if (!m_track_view_delegate->view_event(*event)) {
    QTreeView::keyPressEvent(event);
  }
}

DopeSheetView::TimelineCanvasC::TimelineCanvasC(Animator& animator, DopeSheetView& self)
    : TimelineCanvas(animator, self), m_self(self)
{
}

void DopeSheetView::TimelineCanvasC::update()
{
  m_self.update_second_column();
}

QPoint DopeSheetView::TimelineCanvasC::map_to_global(const QPoint& pos) const
{
  return m_self.viewport()->mapToGlobal(pos);
}

void DopeSheetView::TimelineCanvasC::enable_context_menu()
{
  m_self.setContextMenuPolicy(Qt::PreventContextMenu);
}

void DopeSheetView::TimelineCanvasC::disable_context_menu()
{
  m_self.setContextMenuPolicy(Qt::DefaultContextMenu);
}

QRect DopeSheetView::TimelineCanvasC::track_rect(Track& track)
{
  const QModelIndex index
      = m_self.m_proxy_model->mapFromSource(m_self.m_animator.index(track.property()));
  return m_self.visualRect(index.siblingAtColumn(1));
}

QRect DopeSheetView::TimelineCanvasC::owner_rect(AbstractPropertyOwner& owner)
{
  const QModelIndex index = m_self.m_proxy_model->mapFromSource(m_self.m_animator.index(owner));
  return m_self.visualRect(index.siblingAtColumn(1));
}

}  // namespace omm
