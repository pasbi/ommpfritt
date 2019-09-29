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

TrackViewDelegate::TrackViewDelegate(DopeSheetView& view, Animator& animator)
  : m_view(view)
  , m_animator(animator)
  , m_canvas(animator, *this)
{
}

void TrackViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const
{
  painter->save();
  painter->setClipRect(option.rect);
  painter->setRenderHint(QPainter::HighQualityAntialiasing);

  activate_index(index);
  m_canvas.draw_background(*painter);
  m_canvas.draw_lines(*painter);
  if (m_animator.index_type(index) == Animator::IndexType::Property
     && ::contains(m_expanded_tracks, m_animator.property(index)->track()))
  {
    m_canvas.draw_fcurve(*painter);
  }
  else
  {
    m_canvas.draw_keyframes(*painter);
  }
  painter->restore();
}

QSize TrackViewDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex& index) const
{
  if (m_animator.index_type(index) == Animator::IndexType::Property) {
    Track* track = m_animator.property(index)->track();
    if (::contains(m_expanded_tracks, track)) {
      return QSize(200, 100);
    } else {
      return QSize(200, 10);
    }
  } else {
    return QSize(0, 0);
  }
}

void TrackViewDelegate::mouse_press(QMouseEvent& event)
{
  m_mouse_press_index = m_view.indexAt(event.pos());
  activate_index(m_mouse_press_index);
  m_canvas.mouse_press(event);
}

void TrackViewDelegate::mouse_release(QMouseEvent& event)
{
  activate_index(m_mouse_press_index);
  m_canvas.mouse_release(event);
}

void TrackViewDelegate::mouse_move(QMouseEvent& event)
{
  activate_index(m_mouse_press_index);
  m_canvas.mouse_move(event);
}

void TrackViewDelegate::key_press(QKeyEvent& event)
{
  m_canvas.key_press(event);
}

void TrackViewDelegate::toggle_expanded(const QModelIndex& index)
{
  Track* track = m_animator.property(index)->track();
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
  switch (m_animator.index_type(index)) {
  case Animator::IndexType::Property:
    tracks.insert(m_animator.property(index)->track());
    break;
  case Animator::IndexType::Owner:
    for (Property* p : m_animator.owner(index)->properties().values()) {
      if (p->track() != nullptr) {
        tracks.insert(p->track());
      }
    }
    break;
  default:
    break;
  }

  m_canvas.rect = m_view.visualRect(index);
  m_canvas.tracks = tracks;
}

TrackViewDelegate::TimelineCanvasC::TimelineCanvasC(Animator& animator, TrackViewDelegate& self)
  : TimelineCanvas(animator, 0), m_self(self)
{
}

void TrackViewDelegate::TimelineCanvasC::update()
{
  m_self.m_view.update_second_column();
}

QPoint TrackViewDelegate::TimelineCanvasC::map_to_global(const QPoint& pos) const
{
  return m_self.m_view.viewport()->mapToGlobal(pos);
}

}  // namespace omm
