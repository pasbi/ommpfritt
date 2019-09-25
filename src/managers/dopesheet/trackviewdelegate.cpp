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

namespace omm
{

TrackViewDelegate::TrackViewDelegate(DopeSheetView& view, Animator& animator)
  : m_view(view), m_animator(animator)
{
}

void TrackViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const
{
  painter->fillRect(option.rect, Qt::gray);
  if (m_animator.index_type(index) == Animator::IndexType::Property) {
    Track* track = m_animator.property(index)->track();
    draw_background(*painter, option.rect);
    const auto keyframe_geometry = [this, option](const int frame) {
      const double key_width = pixel_per_frame(option.rect.width());
      return QRectF(QPointF(frame_to_pixel(frame) - key_width/2.0,
                            option.rect.top()),
                    QSizeF(key_width, option.rect.height()));
    };
    for (auto&& [frame, pk] : m_keyframes.at(track)) {
      draw_keyframe(*painter, keyframe_geometry(frame), pk, KeyFrameStyle::Normal);
    }
    for (auto&& [frame, pk] : m_keyframes.at(track)) {
      if (pk.is_dragged) {
        draw_keyframe(*painter, keyframe_geometry(frame + m_current_shift), pk, KeyFrameStyle::Ghost);
      }
    }
  }
  draw_current(*painter, option.rect);
}

QSize TrackViewDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  if (m_animator.index_type(index) == Animator::IndexType::Property) {
    return QSize(200, 10);
  } else {
    return QSize(0, 0);
  }
}

int TrackViewDelegate::start_frame() const
{
  return m_animator.start();
}

int TrackViewDelegate::end_frame() const
{
  return m_animator.end();
}

double TrackViewDelegate::pixel_per_frame(int width) const
{
  return static_cast<double>(width) / static_cast<double>(m_animator.end() - m_animator.start());
}

int TrackViewDelegate::left() const
{
  return m_view.columnViewportPosition(1);
}

int TrackViewDelegate::width() const
{
  return m_view.columnWidth(1);
}

void TrackViewDelegate::remove_keyframe(Track& track, int frame)
{
  m_keyframes[&track].erase(frame);
}

void TrackViewDelegate::insert_keyframe(Track& track, int frame)
{
  m_keyframes[&track][frame] = PersistentKeyFrame {};
}

void TrackViewDelegate::move_keyframe(Track& track, int old_frame, int new_frame)
{
  m_keyframes[&track][new_frame] = m_keyframes[&track].extract(old_frame).mapped();
}

void TrackViewDelegate::remove_track(Track& track)
{
  m_keyframes.erase(&track);
}

void TrackViewDelegate::insert_track(Track& track)
{
  for (int frame : track.key_frames()) {
    m_keyframes[&track][frame] = PersistentKeyFrame {};
  }
}

void TrackViewDelegate::invalidate_cache()
{
  m_keyframes.clear();
  for (const Property* property : m_animator.accelerator().properties()) {
    Track* track = property->track();
    for (int frame : track->key_frames()) {
      m_keyframes[track][frame] = PersistentKeyFrame {};
    }
  }
}

void TrackViewDelegate::deselect_all_keyframes_except(Track* e_track, int e_frame)
{
  for (auto&& [ track, key_frames ] : m_keyframes) {
    for (auto&& [frame, p] : key_frames) {
      if (e_track == track && e_frame == frame) {
        // exception, don't deselect
      } else {
        p.is_selected = false;
        p.is_dragged = false;
      }
    }
  }
}

void TrackViewDelegate
::draw_keyframe(QPainter& painter, const QRectF& rect, const PersistentKeyFrame& k,
                KeyFrameStyle style) const
{
  painter.save();
  switch (style) {
  case KeyFrameStyle::Normal:
    if (k.is_selected) {
      painter.fillRect(rect, Qt::yellow);
    } else {
      painter.fillRect(rect, Qt::red);
    }
    break;
  case KeyFrameStyle::Ghost:
    painter.fillRect(rect, QColor(255, 255, 0, 128));
    break;
  }
  painter.restore();
}

QPoint TrackViewDelegate::map_to_track_view(const QModelIndex& index, const QPoint& point) const
{
  return point - m_view.visualRect(index).topLeft();
}

bool TrackViewDelegate::mouse_button_press(QMouseEvent& event)
{
  m_move_aborted = false;
  m_current_shift = 0;
  const QModelIndex index = m_view.indexAt(event.pos());
  m_mouse_down_pos = event.pos();
  if (m_animator.index_type(index) == Animator::IndexType::Property) {
    Property* property = m_animator.property(index);
    const int frame = pixel_to_frame(event.x());
    auto& key_frames = m_keyframes.at(property->track());
    if (event.modifiers() & Qt::ShiftModifier) {
      if (key_frames.find(frame) != key_frames.end()) {
        key_frames.at(frame).is_selected = !key_frames.at(frame).is_selected;
      }
      m_view.update_second_column();
    } else {
      if (key_frames.find(frame) != key_frames.end()) {
        if (!key_frames.at(frame).is_selected) {
          deselect_all_keyframes_except(property->track(), frame);
          key_frames.at(frame).is_selected = true;
        }
      }
      m_view.update_second_column();
    }
    for (auto&& [track, keyframes] : m_keyframes) {
      for (auto&& [frame, persisten_keyframe] : keyframes) {
        if (persisten_keyframe.is_selected) {
          persisten_keyframe.is_dragged = true;
        }
      }
    }
    return true;
  } else {
    return false;
  }
}

void TrackViewDelegate::mouse_button_release(QMouseEvent& event)
{
  if (m_current_shift == 0) {
    const QModelIndex index = m_view.indexAt(event.pos());
    if (m_animator.index_type(index) == Animator::IndexType::Property) {
      if (!(event.modifiers() & Qt::ShiftModifier)) {
        Track* track = m_animator.property(index)->track();
        const int frame = pixel_to_frame(event.x());
        deselect_all_keyframes_except(track, frame);
      }
    }
  } else if (!m_move_aborted) {
    auto& scene = m_animator.scene;
    const auto keyframes_copy = m_keyframes;
    for (auto&& [track, keyframes] : keyframes_copy) {
      std::set<int> moving_frames;
      for (auto&& [frame, pk] : keyframes) {
        if (pk.is_dragged) {
          m_keyframes.at(track).at(frame).is_dragged = false;
          moving_frames.insert(frame);
        }
      }
      if (!moving_frames.empty()) {
        scene.submit<MoveKeyFrameCommand>(m_animator, track->property(),
                                          moving_frames, m_current_shift);
      }
    }
    m_animator.apply();
  }
  m_view.update_second_column();
}

void TrackViewDelegate::abort_move()
{
  m_move_aborted = true;
  for (auto&& [track, keyframes] : m_keyframes) {
    for (auto&& [frame, pk] : keyframes) {
      if (pk.is_dragged) {
        pk.is_dragged = false;
      }
    }
  }
  m_view.update_second_column();
}

void TrackViewDelegate::mouse_move(QMouseEvent& event)
{
  m_current_shift = pixel_to_frame(event.x()) - pixel_to_frame(m_mouse_down_pos.x());
  m_view.update_second_column();
}

void TrackViewDelegate::draw_background(QPainter& painter, const QRect& rect) const
{
  painter.save();
  QPen pen(Qt::white);
  for (int frame = m_animator.start(); frame <= m_animator.end(); ++frame) {
    double pen_width = 0.0;
    if (frame % 10 == 0) {
      pen_width = 2.0;
    } else if (frame % 2 == 0) {
      if (pixel_per_frame(rect.width()) > 5) {
        pen_width = 1.0;
      }
    } else {
      if (pixel_per_frame(rect.width()) > 10) {
        pen_width = 0.5;
      }
    }
    if (pen_width > 0.0) {
      pen.setWidthF(pen_width);
      painter.setPen(pen);
      const double x = frame_to_pixel(frame);
      painter.drawLine(x, rect.bottom(), x, rect.top());
    }
  }
  painter.restore();
}

void TrackViewDelegate::draw_current(QPainter& painter, const QRect& rect) const
{
  painter.save();
  QPen pen(Qt::black);
  pen.setWidthF(1.0);
  painter.setPen(pen);
  const double x = frame_to_pixel(m_animator.current());
  painter.drawLine(x, rect.bottom(), x, rect.top());
  painter.restore();
}

double TrackViewDelegate::frame_to_pixel(double frame) const
{
  return left() + pixel_per_frame(width()) * (frame - m_animator.start());;
}

int TrackViewDelegate::pixel_to_frame(int pixel) const
{
  return std::round((pixel - left()) / pixel_per_frame(width()) + m_animator.start());
}

}  // namespace omm
