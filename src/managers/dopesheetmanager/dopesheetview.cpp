#include "managers/dopesheetmanager/dopesheetview.h"
#include <KF5/KItemModels/KExtraColumnsProxyModel>
#include <QPainter>
#include <QHeaderView>
#include <QMouseEvent>
#include "managers/dopesheetmanager/trackviewdelegate.h"
#include <memory>
#include "animation/animator.h"
#include "logging.h"
#include "animation/track.h"
#include "properties/property.h"
#include "managers/dopesheetmanager/dopesheetheader.h"
#include "aspects/abstractpropertyowner.h"
#include "proxychain.h"

namespace
{

class ChopProxyModel : public QIdentityProxyModel
{
public:
  int rowCount(const QModelIndex& index) const
  {
    const auto* const animator = this->animator();
    if (animator == nullptr) {
      return 0;
    } else if (animator->index_type(mapToSource(index)) == omm::Animator::IndexType::Property) {
      return 0;
    } else {
      return QIdentityProxyModel::rowCount(index);
    }
  }

  bool hasChildren(const QModelIndex& index) const
  {
    const auto* const animator = this->animator();
    if (animator == nullptr) {
      return 0;
    } else if (animator->index_type(mapToSource(index)) == omm::Animator::IndexType::Property) {
      return false;
    } else {
      return QIdentityProxyModel::hasChildren(index);
    }
  }

private:
  omm::Animator* animator() const { return static_cast<omm::Animator*>(sourceModel()); }
};

class AddColumnProxy : public KExtraColumnsProxyModel
{
public:
  explicit AddColumnProxy()
  {
    appendColumn();
  }

  QVariant
  extraColumnData(const QModelIndex& parent, int row, int extraColumn, int role) const override
  {
    Q_UNUSED(parent)
    Q_UNUSED(row)
    Q_UNUSED(extraColumn)
    Q_UNUSED(role)
    // the extra column displays a delegate which does not rely on data.
    return QVariant();
  }
};

}  // namespace omm

namespace omm
{

DopeSheetView::DopeSheetView(Animator& animator)
  : ItemProxyView<QTreeView>(std::make_unique<ProxyChain>(
      ProxyChain::concatenate<std::unique_ptr<QAbstractProxyModel>>(
        std::make_unique<ChopProxyModel>(),
        std::make_unique<AddColumnProxy>()
    )))
  , m_animator(animator)
  , m_canvas(m_animator, *this)
{
  setModel(&animator);
  setHeader(std::make_unique<DopeSheetHeader>(m_canvas).release());
  auto track_view_delegate = std::make_unique<TrackViewDelegate>(*this, m_canvas);
  m_track_view_delegate = track_view_delegate.get();
  setItemDelegateForColumn(1, track_view_delegate.release());
  connect(&m_animator, SIGNAL(start_changed(int)), this, SLOT(update_second_column()));
  connect(&m_animator, SIGNAL(end_changed(int)), this, SLOT(update_second_column()));
  connect(&m_animator, SIGNAL(current_changed(int)), this, SLOT(update_second_column()));
  connect(&m_animator, SIGNAL(track_changed(Track&)), this, SLOT(update_second_column()));
  connect(&m_canvas, SIGNAL(current_frame_changed(int)), &m_animator, SLOT(set_current(int)));
  setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
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
    const QModelIndex index = indexAt(static_cast<QMouseEvent*>(event)->pos());
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
  const QModelIndex index = m_self.model()->mapFromSource(m_self.m_animator.index(track.property()));
  return m_self.visualRect(index.siblingAtColumn(1));
}

QRect DopeSheetView::TimelineCanvasC::owner_rect(AbstractPropertyOwner& owner)
{
  const QModelIndex index = m_self.model()->mapFromSource(m_self.m_animator.index(owner));
  return m_self.visualRect(index.siblingAtColumn(1));
}

}  // namespace
