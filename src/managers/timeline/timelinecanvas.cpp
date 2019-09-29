#include "managers/timeline/timelinecanvas.h"

#include "animation/track.h"
#include "animation/animator.h"
#include <QCursor>
#include <QMouseEvent>
#include <QKeyEvent>
#include "logging.h"
#include "commands/keyframecommand.h"
#include "scene/scene.h"
#include "scene/history/historymodel.h"

namespace omm
{

TimelineCanvas::TimelineCanvas(Animator& animator, int footer_height)
  : m_animator(animator)
  , m_footer_height(footer_height)
{
  void knot_removed(Track&, int);
  void knot_moved(Track&, int, int);
  connect(&animator, &Animator::knot_removed, this, [this](Track& track, int frame) {
    if (const auto it = m_selection.find(&track); it != m_selection.end()) {
      it->second.erase(frame);
      if (it->second.empty()) {
        m_selection.erase(it);
      }
    }
  });
  connect(&animator, &Animator::knot_moved, this, [this](Track& track, int old_frame, int new_frame)
  {
    if (const auto& it = m_selection.find(&track); it != m_selection.end()) {
      it->second.erase(old_frame);
      it->second.insert(new_frame);
    }
  });
}

void TimelineCanvas::draw(QPainter& painter) const
{
  draw_background(painter);
  draw_lines(painter);
  draw_keyframes(painter);
  draw_current(painter);
}

void TimelineCanvas::draw_background(QPainter& painter) const
{
  painter.save();
  painter.translate(rect.topLeft());
  painter.scale(rect.width(), rect.height());
  const double left = (m_animator.start()-left_frame) * ppf() - ppf()/2.0;
  const double right = (m_animator.end()-left_frame) * ppf() + ppf()/2.0;
  if (left > 0.0) {
    painter.fillRect(QRectF(QPointF(0, 0), QPointF(left, 1.0)), Qt::gray);
  }
  if (right < 1.0) {
    painter.fillRect(QRectF(QPointF(right, 0), QPointF(1.0, 1.0)), Qt::gray);
  }
  if (right > 0.0 && left < 1.0) {
    painter.fillRect(QRectF(QPointF(left, 0.0), QPointF(right, 1.0)), Qt::white);
  }
  painter.restore();
}

void TimelineCanvas::draw_lines(QPainter& painter) const
{
  painter.save();
  painter.translate(rect.topLeft());
  painter.scale(rect.width(), rect.height());

  QPen pen;
  pen.setColor(Qt::black);
  pen.setCosmetic(true);
  painter.setPen(pen);

  for (int frame = left_frame; frame <= right_frame + 1; ++frame) {
    if (ppfs() < 10 && (frame % 2 != 0)) {
      continue;
    } else if (ppfs() < 2 && frame % 10 != 0) {
      continue;
    } else if (ppfs() < 1 && frame % 20 != 0) {
      continue;
    }

    const auto draw_frame_number = [this](int frame) {
      if (ppfs() < 2) {
        return frame % 100 == 0;
      } else if (ppfs() < 10) {
        return frame % 20 == 0;
      } else if (ppfs() < 20) {
        return frame % 10 == 0;
      } else {
        return frame % 2 == 0;
      }
    };

    pen.setWidthF(frame % 10 == 0 ? 2.0 : 1.0);
    painter.setPen(pen);

    const double x = (frame - left_frame) * ppf();
    const double line_start = frame % 2 == 0 ? 0 : 0.05;

    // there is no way in drawing really tiny text. Hence, we must draw the frame numbers in
    // non-normalized coordinates...
    painter.save();
    painter.resetTransform();
    painter.translate(rect.left() + rect.width() * x, rect.top());

    const double line_end = m_footer_height > 0 ? std::max(line_start, footer_y())
                                                : rect.height();
    painter.drawLine(QPointF(0, line_start), QPointF(0, line_end));

    if (m_footer_height > 0 && draw_frame_number(frame)) {
      const QString text = QString("%1").arg(frame);
      const double text_width = QFontMetrics(painter.font()).horizontalAdvance(text);
      painter.drawText(QPointF(-text_width/2.0, rect.height()), text);
    }
    painter.restore();
  }

  painter.restore();
}

void TimelineCanvas::draw_keyframes(QPainter& painter) const
{
  painter.save();
  painter.translate(rect.topLeft());
  const int y = footer_y() / 2.0;

  static const QPainterPath diamond = []() {
    QPainterPath diamond;
    diamond.moveTo(QPointF( 0,  1));
    diamond.lineTo(QPointF( 1,  0));
    diamond.lineTo(QPointF( 0, -1));
    diamond.lineTo(QPointF(-1,  0));
    diamond.closeSubpath();
    return diamond;
  }();

  QPen pen;
  pen.setCosmetic(true);
  pen.setColor(Qt::black);
  const QPointF scale(std::clamp(rect.width() * ppf()/2.0, 4.0, 20.0),
                      std::min(footer_y()/2.0, 20.0));
  pen.setWidthF(std::max(0.0, std::min(scale.x(), scale.y())/5.0));
  painter.setPen(pen);

  const auto draw_keyframe = [scale, &painter, this](int frame, int y, const QColor& color) {
    painter.save();
    painter.translate(frame_to_pixel(frame), y);
    painter.scale(scale.x()*0.8, scale.y()*0.8);
    painter.fillPath(diamond, color);
    painter.drawPath(diamond);
    painter.restore();
  };

  for (int frame = left_frame; frame <= right_frame + 1; ++frame) {
    const bool draw = std::any_of(tracks.begin(), tracks.end(), [frame](const Track* track) {
      return track->has_keyframe(frame);
    });
    if (draw) {
      const bool is_selected = this->is_selected(frame);
      draw_keyframe(frame, y, is_selected ? Qt::yellow : Qt::red);
      if (m_shift != 0 && is_selected) {
        draw_keyframe(frame + m_shift, y, QColor(255, 255, 0, 100));
      }
    }
  }
  painter.restore();
}

void TimelineCanvas::draw_current(QPainter& painter) const
{
  painter.save();
  painter.translate(rect.topLeft());
  const double x = frame_to_pixel(m_animator.current());
  const QRectF current_rect(QPointF(x-ppfs()/2.0, footer_y()),
                            QSizeF(ppfs(), m_footer_height));
  painter.fillRect(current_rect, QColor(255, 128, 0, 60));
  QPen pen;
  pen.setColor(QColor(255, 128, 0, 120));
  pen.setWidthF(4.0);
  pen.setCosmetic(true);
  painter.setPen(pen);
  painter.drawRect(current_rect);
  painter.restore();
}

double TimelineCanvas::ppf() const
{
  return 1.0 / (right_frame - left_frame + 1);
}

double TimelineCanvas::ppfs() const
{
  return rect.width() * ppf();
}

void TimelineCanvas::mouse_press(QMouseEvent& event)
{
  m_mouse_down_pos = event.pos();
  m_last_mouse_pos = event.pos();
  m_pan_active = event.modifiers() & Qt::AltModifier && event.button() == Qt::LeftButton;
  m_zoom_active = event.modifiers() & Qt::AltModifier && event.button() == Qt::RightButton;
  m_shift = 0;
  m_move_aborted = false;
  const int frame = std::round(pixel_to_frame(event.pos().x() - rect.left()));
  if (m_pan_active || m_zoom_active) {
    // do nothing yet
  } else if (event.pos().y() - rect.top() < footer_y()) {
    if (const auto tracks = tracks_at(frame); !tracks.empty()) {
      m_dragging_knots = true;
      if (!(event.modifiers() & Qt::ShiftModifier) && !is_selected(frame)) {
        m_selection.clear();
      }
      select(frame);
      update();
    }
  } else {
    m_dragging_time = true;
    Q_EMIT current_frame_changed(std::round(pixel_to_frame(event.pos().x())));
  }
}

void TimelineCanvas::mouse_move(QMouseEvent& event)
{
  const double min_ppf = 0.5 / rect.width();
  const double max_ppf = 70 / rect.width();
  const QPointF d = QPointF(m_last_mouse_pos - event.pos()) / rect.width();
  if (m_pan_active) {
    QCursor::setPos(map_to_global(m_mouse_down_pos));
    const double min = normalized_to_frame(frame_to_normalized(left_frame) + d.x());
    const double max = normalized_to_frame(frame_to_normalized(right_frame) + d.x());
    left_frame = min;
    right_frame = max;
    update();
  } else if (m_zoom_active) {
    QCursor::setPos(map_to_global(m_mouse_down_pos));
    const double left = (m_mouse_down_pos.x() - rect.left()) / rect.width();
    const double center_frame = normalized_to_frame(left);
    const double right = 1.0 - left;
    double ppf = this->ppf() * std::exp(-d.x() * rect.width() / 300.0);
    ppf = std::clamp(ppf, min_ppf, max_ppf);
    left_frame = center_frame - left / ppf;
    right_frame = center_frame + right  / ppf - 1.0;
    update();
  } else if (m_dragging_knots && !m_move_aborted) {
    m_shift = std::round(pixel_to_frame(event.x()) - pixel_to_frame(m_mouse_down_pos.x()));
    update();
  } else if (m_dragging_time) {
    Q_EMIT current_frame_changed(std::round(pixel_to_frame(event.pos().x() - rect.left())));
  }
}

void TimelineCanvas::mouse_release(QMouseEvent& event)
{
  const int frame = std::round(pixel_to_frame(event.pos().x() - rect.left()));
  if (m_shift == 0 && !m_move_aborted && m_dragging_knots) {
    if (!(event.modifiers() & Qt::ShiftModifier)) {
      m_selection.clear();
    }
    select(frame);
  } else if (!m_move_aborted && m_shift != 0) {
    assert (m_dragging_knots);  // m_shift != 0 imples m_dragging_knots
    std::list<std::unique_ptr<MoveKeyFrameCommand>> commands;
    for (auto&& [track, selected_frames] : m_selection) {
        commands.push_back(std::make_unique<MoveKeyFrameCommand>(m_animator, track->property(),
                                                                 selected_frames, m_shift));
    }
    if (!commands.empty()) {
      auto macro = m_animator.scene.history().start_macro(QObject::tr("Move knots"));
      for (auto&& command : commands) {
        m_animator.scene.submit(std::move(command));
      }
    }
  }
  Q_UNUSED(event)
  m_pan_active = false;
  m_zoom_active = false;
  m_dragging_knots = false;
  m_dragging_time = false;
  m_shift = 0;
  update();
}

void TimelineCanvas::key_press(QKeyEvent& event)
{
  if (event.key() == Qt::Key_Escape) {
    m_move_aborted = true;
    m_shift = 0;
    update();
  }
}

double TimelineCanvas::pixel_to_frame(double pixel) const
{
  return pixel / ppfs() + left_frame;
}

double TimelineCanvas::frame_to_pixel(double frame) const
{
  return (frame - left_frame) * ppfs();
}

double TimelineCanvas::normalized_to_frame(double pixel) const
{
  return pixel / ppf() + left_frame;
}

double TimelineCanvas::frame_to_normalized(double frame) const
{
  return (frame - left_frame) * ppf();
}

std::set<Track*> TimelineCanvas::tracks_at(double frame) const
{
  std::set<Track*> tracks;
  for (Track* track : this->tracks) {
    if (track->has_keyframe(std::round(frame))) {
      tracks.insert(track);
    }
  }
  return tracks;
}

double TimelineCanvas::footer_y() const
{
  return rect.height() - m_footer_height;
}

bool TimelineCanvas::is_selected(int frame) const
{
  return std::any_of(tracks.begin(), tracks.end(), [this, frame](Track* track) {
    if (const auto it = m_selection.find(track); it != m_selection.end()) {
      return ::contains(it->second, frame);
    } else {
      return false;
    }
  });
}

void TimelineCanvas::select(int frame)
{
  for (Track* track : tracks) {
    if (track->has_keyframe(frame)) {
      m_selection[track].insert(frame);
    }
  }
}

}  // namespace omm
