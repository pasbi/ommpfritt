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
#include "preferences/uicolors.h"

namespace omm
{

TimelineCanvas::TimelineCanvas(Animator& animator, QWidget& widget)
  : animator(animator), m_widget(widget)
{
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

void TimelineCanvas::draw_background(QPainter& painter) const
{
  painter.save();
  painter.translate(rect.topLeft());
  painter.scale(rect.width(), rect.height());
  const double left = (animator.start()-left_frame) * ppf() - ppf()/2.0;
  const double right = (animator.end()-left_frame) * ppf() + ppf()/2.0;
  if (left > 0.0) {
    painter.fillRect(QRectF(QPointF(0, 0), QPointF(left, 1.0)),
                     ui_color(m_widget, "TimeLine", "beyond"));
  }
  if (right < 1.0) {
    painter.fillRect(QRectF(QPointF(right, 0), QPointF(1.0, 1.0)),
                     ui_color(m_widget, "TimeLine", "beyond"));
  }
  if (right > 0.0 && left < 1.0) {
    painter.fillRect(QRectF(QPointF(left, 0.0), QPointF(right, 1.0)),
                     ui_color(m_widget, QPalette::Base));
  }
  painter.restore();
}

void TimelineCanvas::draw_lines(QPainter& painter) const
{
  painter.save();

  QPen pen;
  pen.setColor(ui_color(m_widget, "TimeLine", "scale vline"));
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

    pen.setWidthF(frame % 10 == 0 ? 1.0 : 0.5);
    painter.setPen(pen);

    const double x = (frame - left_frame) * ppf();
    const double line_start = frame % 2 == 0 ? 0 : 0.05;

    // there is no way in drawing really tiny text. Hence, we must draw the frame numbers in
    // non-normalized coordinates...
    painter.save();
    painter.resetTransform();
    painter.translate(rect.left() + rect.width() * x, rect.top());

    const double line_end = footer_height > 0 ? std::max(line_start, footer_y()) : rect.height();
    if (line_end != line_start) {
      painter.drawLine(QPointF(0, line_start), QPointF(0, line_end));
    }

    if (footer_height > 0 && draw_frame_number(frame)) {
      const QString text = QString("%1").arg(frame);
      const QFontMetricsF fm(painter.font());
      const double text_width = fm.horizontalAdvance(text);
      const double margin = (footer_height - fm.height()) / 2.0;
      painter.drawText(QPointF(-text_width/2.0, footer_y() + margin + fm.height()), text);
    }
    painter.restore();
  }

  painter.resetTransform();
  if (expanded_track_data != nullptr) {
    const auto& bottom = expanded_track_data->bottom;
    const auto& top = expanded_track_data->top;
    assert(bottom < top);
    const auto& mrect = expanded_track_data->rect_at_mouse_press;
    painter.translate(rect.topLeft());
    const double vspan = top - bottom;
    double logvspan = std::log10(vspan);
    logvspan -= std::fmod(logvspan, 1.0);
    const double spacing = std::pow(10.0, logvspan - 1);
    double y = bottom - std::fmod(bottom, spacing);
    while (y < top) {
      const double py = (y - bottom) / (top - bottom) * mrect.height();
      const double x = mrect.width();
      painter.drawLine(QPointF(0.0, py), QPointF(x, py));
      y += spacing;
    }
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
  pen.setColor(ui_color(m_widget, "TimeLine", "key outline"));
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
      draw_keyframe(frame, y, is_selected
                              ? ui_color(m_widget, "TimeLine", "key selected")
                              : ui_color(m_widget, "TimeLine", "key normal"));
      if (m_shift != 0 && is_selected) {
        draw_keyframe(frame + m_shift, y, ui_color(m_widget, "TimeLine", "key dragged"));
      }
    }
  }
  painter.restore();
}

void TimelineCanvas::draw_current(QPainter& painter) const
{
  painter.save();
  painter.translate(rect.topLeft());
  const double x = frame_to_pixel(animator.current());
  const QRectF current_rect(QPointF(x-ppfs()/2.0, footer_y()),
                            QSizeF(ppfs(), footer_height));
  painter.fillRect(current_rect, ui_color(m_widget, "TimeLine", "slider fill"));
  QPen pen;
  pen.setColor(ui_color(m_widget, "TimeLine", "slider outline"));
  pen.setWidthF(4.0);
  pen.setCosmetic(true);
  painter.setPen(pen);
  painter.drawRect(current_rect);
  painter.restore();
}

void TimelineCanvas::draw_fcurve(QPainter& painter) const
{
  assert(expanded_track_data != nullptr);
  painter.save();
  painter.translate(rect.topLeft());
  const QString top = QString("%1").arg(expanded_track_data->top);
  const QString bottom = QString("%1").arg(expanded_track_data->bottom);
  painter.drawText(QPointF(0.0, painter.fontMetrics().height()), top);
  painter.drawText(QPointF(0.0, rect.height()), bottom);
  for (Track* track : tracks) {
    if (track->type() == "Float") {
      for (int frame = left_frame; frame <= right_frame; ++frame) {
        if (track->has_keyframe(frame)) {
          const double x = rect.width() * (frame - left_frame) * ppf();
          painter.drawLine(QPointF(x, 0), QPointF(x, rect.height()));
        }
      }
    }
  }
  painter.restore();
}

void TimelineCanvas::draw_rubber_band(QPainter& painter) const
{
  if (m_rubber_band_visible) {
    painter.save();
    QPen pen;
    pen.setWidth(2.0);
    pen.setColor(ui_color(m_widget, "TimeLine", "rubberband outline"));
    painter.setPen(pen);
    painter.fillRect(rubber_band(), ui_color(m_widget, "TimeLine", "rubberband fill"));
    painter.drawRect(rubber_band());
    painter.restore();
  }
}

double TimelineCanvas::ppf() const
{
  return 1.0 / (right_frame - left_frame + 1);
}

double TimelineCanvas::ppfs() const
{
  return rect.width() * ppf();
}

bool TimelineCanvas::view_event(QEvent& event)
{
  switch (event.type()) {
  case QEvent::MouseButtonPress:
    return mouse_press(static_cast<QMouseEvent&>(event));
  case QEvent::MouseButtonRelease:
    return mouse_release(static_cast<QMouseEvent&>(event));
  case QEvent::MouseMove:
    return mouse_move(static_cast<QMouseEvent&>(event));
  case QEvent::KeyPress:
    return key_press(static_cast<QKeyEvent&>(event));
  default:
    return false;
  }
}

bool TimelineCanvas::mouse_press(QMouseEvent& event)
{
  m_mouse_down_pos = event.pos();
  m_last_mouse_pos = event.pos();
  m_shift = 0;
  m_move_aborted = false;
  m_pan_active = false;
  m_zoom_active = false;
  const int frame = std::round(pixel_to_frame(event.pos().x() - rect.left()));
  if (event.modifiers() & Qt::AltModifier) {
    m_pan_active = event.button() == Qt::LeftButton;
    m_zoom_active = event.button() == Qt::RightButton;
    disable_context_menu();
    return true;
  } else if (event.pos().y() - rect.top() < footer_y()) {
    if (event.button() == Qt::LeftButton) {
      if (!(event.modifiers() & Qt::ShiftModifier) && !is_selected(frame)) {
        m_selection.clear();
      }
      if (const auto tracks = tracks_at(frame); !tracks.empty()) {
        select(frame);
        update();
        m_dragging_knots = true;
      } else {
        m_rubber_band_visible = true;
        m_rubber_band_origin = event.pos();
        m_rubber_band_corner = m_rubber_band_origin;
      }
      return true;
    }
  } else {
    if (event.button() == Qt::LeftButton) {
      m_dragging_time = true;
      Q_EMIT current_frame_changed(std::round(pixel_to_frame(event.pos().x() - rect.left())));
      return true;
    }
  }
  return false;
}

bool TimelineCanvas::mouse_move(QMouseEvent& event)
{
  const double min_ppf = 0.5 / rect.width();
  const double max_ppf = 70 / rect.width();
  QPointF d = QPointF(m_last_mouse_pos - event.pos()) / rect.width();
  m_last_mouse_pos = event.pos();
  if (expanded_track_data != nullptr) {
    const double dx = std::abs(d.x());
    const double dy = std::abs(d.y());
    if (dx > dy) {
      d.setY(0.0);
    } else if (dx < dy) {
      d.setX(0.0);
    }
  } else {
    d.setY(0.0);
  }
  if (m_pan_active) {
    if (expanded_track_data != nullptr) {
      auto& bottom = expanded_track_data->bottom;
      auto& top = expanded_track_data->top;
      const double dy = d.y() * (top - bottom) * 380.0 / expanded_track_data->rect_at_mouse_press.height();
      LINFO << expanded_track_data->rect_at_mouse_press;
      top += dy;
      bottom += dy;
    }
    const double min = normalized_to_frame(frame_to_normalized(left_frame) + d.x());
    const double max = normalized_to_frame(frame_to_normalized(right_frame) + d.x());
    left_frame = min;
    right_frame = max;
    update();
  } else if (m_zoom_active) {
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
  } else if (m_rubber_band_visible && !m_move_aborted) {
    const QPoint pos = event.pos();
    if (rect.isEmpty()) {
      m_rubber_band_corner = pos;
    } else {
      m_rubber_band_corner = QPoint(std::clamp<int>(pos.x(), rect.left(), rect.right()),
                                    std::clamp<int>(pos.y(), rect.top(), rect.bottom()));
    }

    const int left = pixel_to_frame(rubber_band().left() - rect.left()) + 0.5;
    const int right = pixel_to_frame(rubber_band().right() - rect.left()) + 0.5;

    for (Property* property : animator.accelerator().properties()) {
      Track& track = *property->track();
      if (track_rect(track).intersects(rubber_band())) {
        m_rubber_band_selection[&track].clear();
        for (int frame : track.key_frames()) {
          if (left <= frame && frame <= right) {
            m_rubber_band_selection[&track].insert(frame);
          }
        }
      } else {
        m_rubber_band_selection.erase(&track);
      }
    }
    for (AbstractPropertyOwner* owner : animator.accelerator().owners()) {
      if (owner_rect(*owner).intersects(rubber_band())) {
        for (Property* property : animator.accelerator().properties(*owner)) {
          Track& track = *property->track();
          for (int frame : track.key_frames()) {
            if (left <= frame && frame <= right) {
              m_rubber_band_selection[&track].insert(frame);
            }
          }
        }
      }
    }

    update();
  } else if (m_dragging_time) {
    double x = pixel_to_frame(event.pos().x() - rect.left());
    Q_EMIT current_frame_changed(std::round(x));
  } else {
    return false;
  }
  return true;
}

bool TimelineCanvas::mouse_release(QMouseEvent& event)
{
  m_rubber_band_visible = false;
  for (auto&& [track, selection] : m_rubber_band_selection) {
    m_selection[track].insert(selection.begin(), selection.end());
  }
  m_rubber_band_selection.clear();
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
        commands.push_back(std::make_unique<MoveKeyFrameCommand>(animator, track->property(),
                                                                 selected_frames, m_shift));
    }
    if (!commands.empty()) {
      auto macro = animator.scene.history().start_macro(QObject::tr("Move knots"));
      for (auto&& command : commands) {
        animator.scene.submit(std::move(command));
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
  enable_context_menu();
  return false;
}

bool TimelineCanvas::key_press(QKeyEvent& event)
{
  if (event.key() == Qt::Key_Escape) {
    m_move_aborted = true;
    m_shift = 0;
    update();
    return true;
  } else {
    return false;
  }
}

QRect TimelineCanvas::rubber_band() const
{
  return QRect(m_rubber_band_corner, m_rubber_band_origin).normalized();
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

double TimelineCanvas::normalized_to_value(double y) const
{
  return 0.0;
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
  return rect.height() - footer_height;
}

bool TimelineCanvas::is_selected(int frame) const
{
  const auto relevant_tracks = ::filter_if(tracks, [frame](Track* track) {
    return track->has_keyframe(frame);
  });
  if (relevant_tracks.empty()) {
    return false;
  }

  return std::all_of(relevant_tracks.begin(), relevant_tracks.end(), [this, frame](Track* track) {
    if (auto it = m_selection.find(track); it != m_selection.end()
        && ::contains(it->second, frame))
    {
      return true;
    }
    else if (auto it = m_rubber_band_selection.find(track); it != m_rubber_band_selection.end()
             && ::contains(it->second, frame))
    {
      return true;
    }
    else
    {
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
