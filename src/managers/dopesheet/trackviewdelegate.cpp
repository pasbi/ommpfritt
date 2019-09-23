#include "managers/dopesheet/trackviewdelegate.h"
#include "animation/animator.h"
#include <QPainter>
#include "logging.h"
#include "animation/track.h"
#include "properties/property.h"
#include <algorithm>

namespace
{

void draw_background(QPainter& painter, double left, double right, const QRectF& rect, double ppf)
{
  double x = rect.left();

  painter.save();
  QPen pen(Qt::white);
  for (int frame = left; frame <= right; ++frame) {
    double pen_width = 0.0;
    if (frame % 10 == 0) {
      pen_width = 2.0;
    } else if (frame % 2 == 0) {
      if (ppf > 5) {
        pen_width = 1.0;
      }
    } else {
      if (ppf > 10) {
        pen_width = 0.5;
      }
    }
    if (pen_width > 0.0) {
      pen.setWidthF(pen_width);
      painter.setPen(pen);
      painter.drawLine(x, rect.bottom(), x, rect.top());
    }
    x += ppf;
  }
  painter.restore();
}

void draw_current(QPainter& painter, double left, int current, const QRectF& rect, double ppf)
{
  painter.save();
  QPen pen(Qt::black);
  pen.setWidthF(1.0);
  painter.setPen(pen);
  const double x = (current - left) * ppf + rect.left();
  painter.drawLine(x, rect.bottom(), x, rect.top());
  painter.restore();
}

}  // namespace

namespace omm
{

TrackViewDelegate::TrackViewDelegate(Animator& animator) : m_animator(animator)
{
}

void TrackViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const
{
  painter->fillRect(option.rect, Qt::gray);
  QRectF track_rect = option.rect;
  track_rect.adjust(5, 0, -5, 0);
  const double left = m_animator.start();
  const double right = m_animator.end();
  const double pixel_per_frame = track_rect.width() / (right - left);
  if (m_animator.index_type(index) == Animator::IndexType::Property) {
    Track* track = m_animator.property(index)->track();
    draw_background(*painter, left, right, track_rect, pixel_per_frame);
    const double key_width = std::max(10.0, pixel_per_frame);
    for (int frame : track->key_frames()) {
      const double x = track_rect.left() + (frame - left) * pixel_per_frame;
      const QRectF key_rect(QPointF(x - key_width/2.0, track_rect.top()),
                            QSizeF(key_width, track_rect.height()));
      draw_keyframe(*painter, key_rect, m_keyframes.at(track).at(frame));
    }
  }
  draw_current(*painter, left, m_animator.current(), track_rect, pixel_per_frame);
}

QSize TrackViewDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  if (m_animator.index_type(index) == Animator::IndexType::Property) {
    return QSize(200, 10);
  } else {
    return QSize(0, 0);
  }
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

void TrackViewDelegate
::draw_keyframe(QPainter& painter, const QRectF& rect, const PersistentKeyFrame& k) const
{
  painter.save();
  painter.fillRect(rect, Qt::red);
  painter.restore();
}

}  // namespace omm
