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

  activate_index(index);
  m_canvas.draw(*painter);
  painter->restore();
}

QSize TrackViewDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex& index) const
{
  if (m_animator.index_type(index) == Animator::IndexType::Property) {
    return QSize(200, 10);
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

void TrackViewDelegate::activate_index(const QModelIndex& index) const
{
  std::set<Track*> tracks;
  if (m_animator.index_type(index) == Animator::IndexType::Property) {
    Track* track = m_animator.property(index)->track();
    tracks.insert(track);
  }
  m_canvas.rect = m_view.visualRect(index);
  m_canvas.tracks = tracks;
}

TrackViewDelegate::TimelineCanvasC::TimelineCanvasC(Animator& animator, TrackViewDelegate& self)
  : TimelineCanvas(animator, 0)
  , m_self(self)
{
}

void TrackViewDelegate::TimelineCanvasC::update()
{
  m_self.m_view.update_second_column();
}

QPoint TrackViewDelegate::TimelineCanvasC::map_to_global(const QPoint& pos) const
{
  return m_self.m_view.mapToGlobal(pos);
}


}  // namespace omm
