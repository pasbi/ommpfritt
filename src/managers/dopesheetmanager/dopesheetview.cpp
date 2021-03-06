#include "managers/dopesheetmanager/dopesheetview.h"
#include "animation/animator.h"
#include "animation/track.h"
#include "aspects/abstractpropertyowner.h"
#include "logging.h"
#include "managers/dopesheetmanager/dopesheetheader.h"
#include "managers/dopesheetmanager/trackviewdelegate.h"
#include "properties/property.h"
#include "proxychain.h"
#include <KF5/KItemModels/KExtraColumnsProxyModel>
#include <QHeaderView>
#include <QMouseEvent>
#include <QPainter>
#include <memory>

namespace
{
class ChopProxyModel : public QIdentityProxyModel
{
public:
  [[nodiscard]] int rowCount(const QModelIndex& index) const override
  {
    using namespace omm;
    const auto* const animator = this->animator();
    if (animator == nullptr
        || Animator::index_type(mapToSource(index)) == Animator::IndexType::Property) {
      return 0;
    } else {
      return QIdentityProxyModel::rowCount(index);
    }
  }

  [[nodiscard]] bool hasChildren(const QModelIndex& index) const override
  {
    using namespace omm;
    const auto* const animator = this->animator();
    if (animator == nullptr
        || Animator::index_type(mapToSource(index)) == Animator::IndexType::Property) {
      return false;
    } else {
      return QIdentityProxyModel::hasChildren(index);
    }
  }

private:
  [[nodiscard]] omm::Animator* animator() const
  {
    return dynamic_cast<omm::Animator*>(sourceModel());
  }
};

class AddColumnProxy : public KExtraColumnsProxyModel
{
public:
  explicit AddColumnProxy()
  {
    appendColumn();
  }

  [[nodiscard]] QVariant extraColumnData([[maybe_unused]] const QModelIndex& parent,
                                         [[maybe_unused]] int row,
                                         [[maybe_unused]] int extraColumn,
                                         [[maybe_unused]] int role) const override
  {
    // the extra column displays a delegate which does not rely on data.
    return QVariant();
  }
};

}  // namespace

namespace omm
{
DopeSheetView::DopeSheetView(Animator& animator)
    : ItemProxyView<QTreeView>(
        std::make_unique<ProxyChain>(ProxyChain::concatenate<std::unique_ptr<QAbstractProxyModel>>(
            std::make_unique<ChopProxyModel>(),
            std::make_unique<AddColumnProxy>()))),
      m_animator(animator), m_canvas(m_animator, *this)
{
  setModel(&animator);
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
  const QModelIndex index
      = m_self.model()->mapFromSource(m_self.m_animator.index(track.property()));
  return m_self.visualRect(index.siblingAtColumn(1));
}

QRect DopeSheetView::TimelineCanvasC::owner_rect(AbstractPropertyOwner& owner)
{
  const QModelIndex index = m_self.model()->mapFromSource(m_self.m_animator.index(owner));
  return m_self.visualRect(index.siblingAtColumn(1));
}

}  // namespace omm
