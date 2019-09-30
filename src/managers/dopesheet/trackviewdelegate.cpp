#include "managers/dopesheet/trackviewdelegate.h"
#include "animation/animator.h"
#include <QPainter>
#include "logging.h"
#include "animation/track.h"
#include "properties/property.h"
#include <QEvent>
#include <QMouseEvent>
#include <algorithm>
#include "managers/dopesheet/dopesheetview.h"
#include "scene/scene.h"
#include "scene/history/historymodel.h"
#include "commands/keyframecommand.h"
#include "scene/messagebox.h"
#include "managers/timeline/slider.h"
#include "animation/track.h"

namespace omm
{

TrackViewDelegate::TrackViewDelegate(DopeSheetView& view, TimelineCanvas& canvas)
  : m_view(view)
  , m_canvas(canvas)
{
}

void TrackViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const
{
  painter->save();
  painter->setClipRect(option.rect);
  painter->setRenderHint(QPainter::HighQualityAntialiasing);

  activate_index(index);
  const bool is_property = m_canvas.animator.index_type(index) == Animator::IndexType::Property;

  m_canvas.draw_background(*painter);

  if (!is_property) {
    painter->save();
    QPen pen;
    pen.setColor(Qt::black);
    pen.setWidth(4);
    painter->setPen(pen);
    painter->drawLine(option.rect.topLeft(), option.rect.topRight());
    painter->restore();
    painter->fillRect(option.rect, QColor(0, 0, 255, 80));
  }

  m_canvas.draw_lines(*painter);
  if (is_property && ::contains(m_expanded_tracks, m_canvas.animator.property(index)->track())) {
    m_canvas.draw_fcurve(*painter);
  } else {
    m_canvas.draw_keyframes(*painter);
  }

  painter->restore();
}

QSize TrackViewDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex& index) const
{
  if (m_canvas.animator.index_type(index) == Animator::IndexType::Property) {
    Track* track = m_canvas.animator.property(index)->track();
    if (::contains(m_expanded_tracks, track)) {
      return QSize(200, 100);
    } else {
      return QSize(200, 10);
    }
  } else {
    return QSize(0, 0);
  }
}

bool TrackViewDelegate::view_event(QEvent& event)
{
  return m_canvas.view_event(event);
}

void TrackViewDelegate::toggle_expanded(const QModelIndex& index)
{
  Track* track = m_canvas.animator.property(index)->track();
  if (const auto it = m_expanded_tracks.find(track); it != m_expanded_tracks.end()) {
    m_expanded_tracks.erase(it);
    Q_EMIT sizeHintChanged(index);
  } else if (track->is_numerical()) {
    m_expanded_tracks.insert(track);
    Q_EMIT sizeHintChanged(index);
  }
}

void TrackViewDelegate::activate_index(const QModelIndex& index) const
{
  std::set<Track*> tracks;
  switch (m_canvas.animator.index_type(index)) {
  case Animator::IndexType::Property:
    tracks.insert(m_canvas.animator.property(index)->track());
    break;
  case Animator::IndexType::Owner:
    for (Property* p : m_canvas.animator.owner(index)->properties().values()) {
      if (p->track() != nullptr) {
        tracks.insert(p->track());
      }
    }
    break;
  default:
    break;
  }

  if (index.isValid()) {
    m_canvas.rect = m_view.visualRect(index);
  } else {
    m_canvas.rect = QRectF(QPointF(m_view.columnViewportPosition(1), 0),
                           QSizeF(m_view.columnWidth(1), 0));;
  }
  m_canvas.footer_height = 0;
  m_canvas.tracks = tracks;
}

}  // namespace omm
